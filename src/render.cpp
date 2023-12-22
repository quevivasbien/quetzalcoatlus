#include <algorithm>
#include <array>
#include <cassert>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

#include "color/color.hpp"
#include "render.hpp"

// if defined, run in multithreaded mode, helpful to disable when debugging
#define MULTITHREADED

// the number of pixels given to a single thread at a time
const size_t THREAD_JOB_SIZE = 4096;

struct PixelSample {
    SpectrumSample color;
    Vec3 normal;
    SpectrumSample albedo;

    PixelSample() : color(0.0f), normal(0.0f, 0.0f, 0.0f), albedo(0.0f) {}
    PixelSample(
        const Vec3& normal,
        const SpectrumSample& albedo
    ) : color(0.0f), normal(normal), albedo(albedo) {}

    PixelSample& operator+=(const PixelSample& other) {
        color += other.color;
        normal += other.normal;
        albedo += other.albedo;
        return *this;
    }

    PixelSample& operator/=(float c) {
        color /= c;
        normal /= c;
        albedo /= c;
        return *this;
    }
};

float power_heuristic(int n_f, float f_pdf, int n_g, float g_pdf) {
    float f = n_f * f_pdf;
    float g = n_g * g_pdf;
    if (std::isinf(f * f)) {
        return 1.0f;
    }
    return f * f / (f * f + g * g);
}

SpectrumSample sample_lights(
    const Scene& scene,
    const SurfaceInteraction& si,
    const BSDF& bsdf,
    const WavelengthSample& wavelengths,
    Sampler& sampler
) {
    auto [light, sample_proba] = scene.sample_lights(si.point, si.normal, sampler);
    // need to do this regardless of early exit in order to keep sampler depth even
    Vec2 sample2 = sampler.sample_2d();
    if (!light) {
        sampler.sample_2d();  
        return SpectrumSample(0.0f);
    }
    auto ls = light->sample(si, wavelengths, sample2);
    if (!ls || ls->spec.is_zero() || ls->pdf == 0.0f) {
        return SpectrumSample(0.0f);
    }
    Vec3 wo = si.wo;
    Vec3 wi = ls->wi;
    auto f = bsdf(wo, wi) * std::abs(wi.dot(si.normal));
    if (f.is_zero() || scene.occluded(si.point, ls->p_light)) {
        return SpectrumSample(0.0f);
    }
    float p_l = sample_proba * ls->pdf;
    if (light->type() == LightType::AREA) {
        float p_b = bsdf.pdf(wo, wi);
        float w_l = power_heuristic(1, p_l, 1, p_b);
        return ls->spec * f * (w_l / p_l);
    }
    else {
        return ls->spec * f / p_l;
    }
}

PixelSample sample_pixel(
    Ray ray,
    const Scene& scene,
    WavelengthSample& wavelengths,
    Sampler& sampler,
    size_t max_bounces
) {
    PixelSample pxs{};
    SpectrumSample weight(1.0f);
    size_t depth = 0;
    bool specular_bounce = false;
    bool any_nonspecular_bounce = false;
    float p_b = 1.0f;
    float ior_scale = 1.0f;
    Pt3 last_p;
    Vec3 last_normal;
    while (!weight.is_zero()) {
        auto si = scene.ray_intersect(ray, wavelengths, sampler);
        // no intersection, add background and break
        if (!si) {
            break;
        }
        if (depth == 0) {
            pxs.normal = si->normal;
        }
        auto emitted = si->emission(-ray.d, wavelengths);
        if (!emitted.is_zero()) {
            if (depth == 0 || specular_bounce) {
                pxs.color += weight * emitted;
            }
            else {
                // compute importance-sampled weight for area light
                auto light = si->light;
                assert(light);
                float light_proba = scene.light_sample_pmf(last_p, last_normal, light)
                    * light->pdf(last_p, ray.d);
                float light_weight = power_heuristic(1, p_b, 1, light_proba);

                pxs.color += emitted * (weight * light_weight);
            }
        }
        if (depth == max_bounces) {
            break;
        }

        auto bsdf = si->bsdf(ray, wavelengths, sampler.sample_1d());
        if (!bsdf) {
            // I think this might mess up sample depth
            // This should never happen now, but is something to pay attention to in the future
            ray = si->skip_intersection(ray);
            continue;
        }
        
        if (!bsdf->is_specular()) {
            // sample direct illumination from light sources
            pxs.color += weight * sample_lights(scene, *si, *bsdf, wavelengths, sampler);
        }

        auto bsdf_sample = bsdf->sample(si->wo, sampler.sample_1d(), sampler.sample_2d());
        if (!bsdf_sample) {
            break;
        }
        if (depth == 0) {
            pxs.albedo = bsdf_sample->spec;
        }

        weight *= bsdf_sample->spec * std::abs(bsdf_sample->wi.dot(si->normal)) / bsdf_sample->pdf;

        p_b = bsdf_sample->pdf_is_proportional ? bsdf->pdf(si->wo, bsdf_sample->wi) : bsdf_sample->pdf;
        specular_bounce = bsdf_sample->scatter_type.specular;
        any_nonspecular_bounce |= !specular_bounce;
        if (bsdf_sample->scatter_type.transmission) {
            ior_scale *= bsdf_sample->ior;
        }
        last_p = si->point;
        last_normal = si->normal;

        ray = Ray(si->point, bsdf_sample->wi);
        depth++;

        // maybe terminate early (russian roulette)
        float roulette_sample = sampler.sample_1d();
        auto rr_weight = weight * ior_scale;
        if (rr_weight.max_component() < 1.f && depth > 1) {
            float q = std::max(0.0f, 1.0f - rr_weight.max_component());
            if (roulette_sample < q) {
                break;
            }
            weight /= 1.0f - q;
        }
    }

    return pxs;
}

void render_pixels(
    const Camera& camera,
    const Scene& scene,
    Sampler& sampler,
    size_t max_bounces,
    RenderResult& result,
    size_t start_index,
    size_t end_index,
    size_t& global_index,
    std::mutex& mutex
) {
    int n_samples = sampler.samples_per_pixel();
    for (size_t i = start_index; i < end_index; i++) {
        size_t x = i % camera.image_width;
        size_t y = camera.image_height - i / camera.image_width - 1;

        RGB color{};
        Vec3 normal{};
        RGB albedo{};
        for (size_t s = 0; s < n_samples; s++) {
            sampler.start_pixel_sample(x, y, s);
            auto jitter = sampler.sample_pixel();
            float u = float(x) + jitter.x;
            float v = float(y) + jitter.y;
            Ray r = camera.cast_ray(u, v);
            WavelengthSample wavelengths = WavelengthSample::uniform(sampler.sample_1d());
            auto pxs = sample_pixel(r, scene, wavelengths, sampler, max_bounces);
            color += camera.sensor.to_sensor_rgb(pxs.color, wavelengths);
            albedo += camera.sensor.to_sensor_rgb(pxs.albedo, wavelengths);
            normal += pxs.normal;
        }
        
        color /= float(n_samples);
        albedo /= float(n_samples);
        normal /= float(n_samples);

        result.color_buffer[i * 3 + 0] = color.x;
        result.color_buffer[i * 3 + 1] = color.y;
        result.color_buffer[i * 3 + 2] = color.z;

        result.normal_buffer[i * 3 + 0] = normal.x;
        result.normal_buffer[i * 3 + 1] = normal.y;
        result.normal_buffer[i * 3 + 2] = normal.z;

        result.albedo_buffer[i * 3 + 0] = albedo.x;
        result.albedo_buffer[i * 3 + 1] = albedo.y;
        result.albedo_buffer[i * 3 + 2] = albedo.z;
    }

    {
        std::lock_guard<std::mutex> lock(mutex);
        if (global_index == camera.image_height * camera.image_width) {
            return;
        }
        start_index = global_index;
        global_index = std::min(global_index + THREAD_JOB_SIZE, camera.image_height * camera.image_width);
        end_index = global_index;
    }
    render_pixels(
        camera,
        scene,
        sampler,
        max_bounces,
        result,
        start_index,
        end_index,
        global_index,
        mutex
    );
}

RenderResult render(
    const Camera& camera,
    const Scene& scene,
    size_t n_samples,
    size_t max_bounces
) {
    RenderResult result(camera.image_height, camera.image_width);
    if (!scene.ready()) {
        std::cout << "Scene must be committed before rendering." << std::endl;
        return result;
    }
    
    size_t image_size = result.width * result.height;
    Sampler sampler(n_samples, camera.image_width, camera.image_height, 0);

    #ifdef MULTITHREADED
    std::vector<std::thread> threads;
    size_t n_threads = std::clamp<size_t>(
        std::thread::hardware_concurrency(),
        1, (image_size + THREAD_JOB_SIZE - 1) / THREAD_JOB_SIZE
    );
    std::cout << "Using " << n_threads << " threads" << std::endl;

    // make a copy of the sampler for each thread
    std::vector<Sampler> samplers;
    samplers.reserve(n_threads);
    for (size_t t = 0; t < n_threads; t++) {
        samplers.push_back(sampler);
    }

    size_t end_index = 0;
    std::mutex mutex;
    for (size_t t = 0; t < n_threads; t++) {
        size_t start_index = end_index;
        {
            std::lock_guard<std::mutex> lock(mutex);
            end_index += THREAD_JOB_SIZE;
            if (end_index > image_size) {
                end_index = image_size;
            }
        }

        threads.push_back(std::thread(
            render_pixels,
            std::ref(camera),
            std::ref(scene),
            std::ref(samplers[t]),
            max_bounces,
            std::ref(result),
            start_index,
            end_index,
            std::ref(end_index),
            std::ref(mutex)
        ));
    }
    for (auto& t : threads) {
        t.join();
    }
    #else
    // Single threaded render
    std::mutex _mutex;
    render_pixels(
        camera, scene, sampler, max_bounces,
        result, 0, image_size, image_size, _mutex
    );
    #endif

    return result;
}
