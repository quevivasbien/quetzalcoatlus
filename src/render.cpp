#include <algorithm>
#include <array>
#include <cassert>
#include <chrono>
#include <iostream>
#include <mutex>
#include <random>
#include <thread>
#include <vector>

#include "color/color.hpp"
#include "render.hpp"

// if defined, run in multithreaded mode, helpful to disable when debugging
#define MULTITHREADED

// the number of pixels given to a single thread at a time
const size_t THREAD_JOB_SIZE = 4096;

PixelSample& PixelSample::operator+=(const PixelSample& other) {
    color += other.color;
    normal += other.normal;
    albedo += other.albedo;
    return *this;
}

PixelSample& PixelSample::operator/=(float c) {
    color /= c;
    normal /= c;
    albedo /= c;
    return *this;
}

float power_heuristic(int n_f, float f_pdf, int n_g, float g_pdf) {
    float f = n_f * f_pdf;
    float g = n_g * g_pdf;
    if (std::isinf(f * f)) {
        return 1.0f;
    }
    return f * f / (f * f + g * g);
}

Renderer::Renderer(
    const Camera& camera,
    const Scene& scene,
    size_t n_samples,
    size_t max_bounces
) :
    camera(camera),
    scene(scene),
    n_samples(n_samples),
    max_bounces(max_bounces),
    result(camera.image_height, camera.image_width),
    progress_bar { .total = camera.image_height * camera.image_width },
    use_media(camera.medium || scene.has_media())
{}

RenderResult& Renderer::render() {
    if (!scene.ready()) {
        std::cout << "Scene must be committed before rendering." << std::endl;
        return result;
    }

    size_t image_size = result.width * result.height;
    auto start_time = std::chrono::steady_clock::now();

    Sampler sampler(n_samples, camera.image_width, camera.image_height, 0);

    #ifdef MULTITHREADED
    std::vector<std::thread> threads;
    size_t n_threads = std::clamp<size_t>(
        std::thread::hardware_concurrency(),
        1, (image_size + THREAD_JOB_SIZE - 1) / THREAD_JOB_SIZE
    );
    std::cout << "Rendering with " << n_threads << " threads" << std::endl;

    // make a copy of the sampler for each thread
    std::vector<Sampler> samplers;
    samplers.reserve(n_threads);
    for (size_t t = 0; t < n_threads; t++) {
        samplers.push_back(sampler);
    }

    for (size_t t = 0; t < n_threads; t++) {
        size_t start_index = current_index;
        {
            std::lock_guard<std::mutex> lock(mutex);
            current_index += THREAD_JOB_SIZE;
            if (current_index > image_size) {
                current_index = image_size;
            }
        }

        threads.push_back(std::thread(
            &Renderer::render_pixels,
            this,
            start_index,
            current_index,
            std::ref(samplers[t])
        ));
    }
    for (auto& t : threads) {
        t.join();
    }
    #else
    // Single threaded render
    std::cout << "Rendering in single threaded mode" << std::endl;
    render_pixels(
        0, image_size, std::ref(sampler)
    );
    #endif

    auto end_time = std::chrono::steady_clock::now();
    std::chrono::duration<float> duration = end_time - start_time;
    std::cout << std::endl << "Render time: " << std::fixed << std::setprecision(3) << duration << std::endl;

    return result;
}

void Renderer::render_pixels(
    size_t start_index,
    size_t end_index,
    Sampler& sampler
) {
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
            PixelSample pxs;
            if (use_media) {
                pxs = sample_pixel_with_media(r, wavelengths, sampler);
            }
            else {
                pxs = sample_pixel(r, wavelengths, sampler);
            }
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
    progress_bar.increment(end_index - start_index);

    {
        std::lock_guard<std::mutex> lock(mutex);
        if (current_index == camera.image_height * camera.image_width) {
            return;
        }
        start_index = current_index;
        current_index = std::min(current_index + THREAD_JOB_SIZE, camera.image_height * camera.image_width);
        end_index = current_index;
    }
    render_pixels(
        start_index,
        end_index,
        sampler
    );
}

void add_bg_light(
    SpectrumSample& color,
    const Scene& scene,
    const SpectrumSample& weight,
    const WavelengthSample& wavelengths
) {
    auto bg_light = scene.get_bg_light();
    if (bg_light.spectrum) {
        color += weight * SpectrumSample::from_spectrum(*bg_light.spectrum, wavelengths) * bg_light.scale;
    }
}

SpectrumSample sample_albedo(const SurfaceInteraction& si, const BSDF& bsdf) {
    const size_t N_ALBEDO_SAMPLES = 16;
    const std::array<float, N_ALBEDO_SAMPLES> uc = {
        0.75741637, 0.37870818, 0.7083487, 0.18935409, 0.9149363, 0.35417435,
        0.5990858,  0.09467703, 0.8578725, 0.45746812, 0.686759,  0.17708716,
        0.9674518,  0.2995429,  0.5083201, 0.047338516
    };
    const std::array<Vec2, N_ALBEDO_SAMPLES> u2 = {
        Vec2(0.855985, 0.570367), Vec2(0.381823, 0.851844),
        Vec2(0.285328, 0.764262), Vec2(0.733380, 0.114073),
        Vec2(0.542663, 0.344465), Vec2(0.127274, 0.414848),
        Vec2(0.964700, 0.947162), Vec2(0.594089, 0.643463),
        Vec2(0.095109, 0.170369), Vec2(0.825444, 0.263359),
        Vec2(0.429467, 0.454469), Vec2(0.244460, 0.816459),
        Vec2(0.756135, 0.731258), Vec2(0.516165, 0.152852),
        Vec2(0.180888, 0.214174), Vec2(0.898579, 0.503897)
    };

    return bsdf.rho_hd(si.wo, uc, u2);
}

// decide whether to terminate early, and update the sampling weight appropriately
bool russian_roulette(
    SpectrumSample& weight,
    float ior_scale,
    size_t depth,
    float roulette_sample,
    float scale_correction = 1.0f
) {
    auto rr_weight = weight * ior_scale / scale_correction;
    if (rr_weight.max_component() < 1.f && depth > 0) {
        float q = std::max(0.0f, 1.0f - rr_weight.max_component());
        if (roulette_sample < q) {
            return true;
        }
        weight /= 1.0f - q;
    }
    return false;
}

PixelSample Renderer::sample_pixel(
    Ray ray,
    WavelengthSample& wavelengths,
    Sampler& sampler
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
        auto si_opt = scene.ray_intersect(ray);
        if (!si_opt) {
            add_bg_light(pxs.color, scene, weight, wavelengths);
            break;
        }
        
        auto& si = *si_opt;

        if (depth == 0) {
            // collect normal for first surface interaction
            pxs.normal = si.normal;
        }

        // add emitted light if intersected object is an area light
        auto emitted = si.emission(-ray.d, wavelengths);
        if (!emitted.is_zero()) {
            if (depth == 0 || specular_bounce) {
                pxs.color += weight * emitted;
            }
            else {
                // compute importance-sampled weight for area light
                auto light = si.light;
                float light_proba = scene.light_sample_pmf(last_p, last_normal, light)
                    * light->pdf(last_p, ray.d);
                float light_weight = power_heuristic(1, p_b, 1, light_proba);

                pxs.color += emitted * (weight * light_weight);
            }
        }
        
        // check if we've reached max bounces
        if (depth == max_bounces) {
            break;
        }

        auto bsdf_opt = si.bsdf(ray, wavelengths, sampler.sample_1d());
        if (!bsdf_opt) {
            // we hit a surface, but that surface has no material properties
            specular_bounce = true;
            ray = si.skip_intersection(ray);
            continue;
        }
        auto& bsdf = *bsdf_opt;

        if (depth == 0) {
            // collect albedo for first surface interaction (with a sampleable albedo)
            pxs.albedo = sample_albedo(si, bsdf);
        }

        if (!bsdf.is_specular()) {
            // sample direct illumination from light sources
            pxs.color += weight * sample_direct_lighting(si, bsdf, wavelengths, sampler);
        }

        auto bsdf_sample_opt = bsdf.sample(
            si.wo,
            sampler.sample_1d(),
            sampler.sample_2d()
        );
        if (!bsdf_sample_opt) {
            break;
        }
        auto& bsdf_sample = *bsdf_sample_opt;

        weight *= bsdf_sample.spec * std::abs(bsdf_sample.wi.dot(si.normal)) / bsdf_sample.pdf;

        p_b = bsdf_sample.pdf_is_proportional ? bsdf.pdf(si.wo, bsdf_sample.wi) : bsdf_sample.pdf;
        specular_bounce = bsdf_sample.scatter_type.specular;
        any_nonspecular_bounce |= !specular_bounce;
        if (bsdf_sample.scatter_type.transmission) {
            ior_scale *= bsdf_sample.ior;
        }
        last_p = si.point;
        last_normal = si.normal;

        // spawn new ray
        ray = Ray(si.point, bsdf_sample.wi);

        if (russian_roulette(weight, ior_scale, depth, sampler.sample_1d())) {
            break;
        }

        depth++;
    }

    return pxs;
}

// the heavy lifting goes on here
// computes a single sample on a single pixel
PixelSample Renderer::sample_pixel_with_media(
    Ray ray,
    WavelengthSample& wavelengths,
    Sampler& sampler
) {
    PixelSample pxs{};
    SpectrumSample weight(1.0f);
    size_t depth = 0;
    bool specular_bounce = false;
    bool any_nonspecular_bounce = false;
    // rescaled unidirectional path probability
    SpectrumSample r_u(1.0f);
    // rescaled light path probability
    SpectrumSample r_l(1.0f);
    float ior_scale = 1.0f;
    Pt3 last_p;
    Vec3 last_normal;

    while (!weight.is_zero()) {
        auto si = scene.ray_intersect(ray);
        if (ray.medium) {
            // handle scattered rays
            bool scattered = false;
            bool terminated = false;
            float t_max = si ? si->t : std::numeric_limits<float>::infinity();
            // initialize rng for transmittance
            std::mt19937 rng(hash(sampler.sample_1d()));
            std::uniform_real_distribution<float> dist(0.0f, 1.0f);
            auto maj = sample_majorant(
                ray,
                t_max,
                sampler.sample_1d(),
                rng,
                wavelengths,
                [&](Pt3 p, const MediumSample& ms, const SpectrumSample& maj_val, const SpectrumSample& maj) {
                    // handle scattering event
                    float p_absorption = ms.absorption[0] / maj_val[0];
                    float p_scattering = ms.scattering[0] / maj_val[0];
                    float p_null = std::max(0.0f, 1.0f - p_absorption - p_scattering);
                    float u = dist(rng);
                    int mode = Sampler::sample_discrete(u, {p_absorption, p_scattering, p_null});
                    if (mode == 0) {
                        // handle absorption
                        terminated = true;
                        return false;
                    }
                    else if (mode == 1) {
                        // handle scattering
                        if (depth >= max_bounces) {
                            terminated = true;
                            return false;
                        }
                        float pdf = maj[0] * ms.scattering[0];
                        weight *= maj * ms.scattering / pdf;
                        r_u *= maj * ms.scattering / pdf;
                        if (!weight.is_zero() && !r_u.is_zero()) {
                            MediumInteraction intr {
                                .point = p,
                                .wo = -ray.d,
                                .medium = ray.medium,
                                .phase = ms.phase
                            };
                            // sample direct lighting
                            pxs.color += sample_direct_lighting_with_media(
                                intr,
                                wavelengths,
                                r_u,
                                sampler
                            ) * weight;
                            // sample new direction
                            auto ps_opt = intr.phase.sample(-ray.d, sampler.sample_2d());
                            if (!ps_opt || ps_opt->pdf == 0.0f) {
                                terminated = true;
                            }
                            else {
                                auto ps = *ps_opt;
                                // weight *= ps.pdf / ps.pdf;  // not necessary since I've only implemented the HG phase func
                                r_l = r_u / ps.pdf;
                                last_p = p;
                                // TODO: what to do with last_normal?
                                scattered = true;
                                ray.o = p;
                                ray.d = ps.wi;
                                specular_bounce = false;
                                any_nonspecular_bounce = true;
                            }
                        }
                        depth++;
                        return false;
                    }
                    else {
                        // handle null event
                        auto null_value = (maj_val - ms.absorption - ms.scattering).map([](float v) { return std::max(0.0f, v); });
                        float pdf = maj[0] * null_value[0];
                        weight *= maj * null_value / pdf;
                        if (pdf == 0.0f) {
                            weight = SpectrumSample(0.0f);
                        }
                        r_u *= maj * null_value / pdf;
                        r_l *= maj * maj_val / pdf;
                        return !(weight.is_zero() || r_u.is_zero());
                    }
                }
            );
            if (terminated || weight.is_zero() || r_u.is_zero()) {
                return pxs;
            }
            if (scattered) {
                continue;
            }

            weight *= maj / maj[0];
            r_u *= maj / maj[0];
            r_l *= maj / maj[0];
        }

        // handle unscattered rays
        if (!si) {
            // no intersection, add background and break
            add_bg_light(pxs.color, scene, weight, wavelengths);
            break;
        }
        if (depth == 0) {
            pxs.normal = si->normal;
        }
        // add emitted light if intersected object is an area light
        auto emitted = si->emission(-ray.d, wavelengths);
        if (!emitted.is_zero()) {
            if (depth == 0 || specular_bounce) {
                pxs.color += weight * emitted / r_u.average();
            }
            else {
                // compute importance-sampled weight for area light
                auto light = si->light;
                float light_proba = scene.light_sample_pmf(last_p, last_normal, light)
                    * light->pdf(last_p, ray.d);
                r_l *= light_proba;
                pxs.color += emitted * (weight / (r_u + r_l).average());
            }
        }

        auto bsdf = si->bsdf(ray, wavelengths, sampler.sample_1d());
        if (!bsdf) {
            // I think this might mess up sample depth
            // This should never happen now, but is something to pay attention to in the future
            specular_bounce = true;
            ray = si->skip_intersection(ray);
            continue;
        }

        if (depth == 0) {
            pxs.albedo = sample_albedo(*si, *bsdf);
        }

        if (depth >= max_bounces) {
            break;
        }
        
        if (!bsdf->is_specular()) {
            // sample direct illumination from light sources
            pxs.color += weight * sample_direct_lighting_with_media(*si, *bsdf, wavelengths, r_u,sampler);
        }

        last_p = si->point;
        last_normal = si->normal;

        auto bsdf_sample = bsdf->sample(si->wo, sampler.sample_1d(), sampler.sample_2d());
        if (!bsdf_sample) {
            break;
        }

        weight *= bsdf_sample->spec * std::abs(bsdf_sample->wi.dot(si->normal)) / bsdf_sample->pdf;

        if (bsdf_sample->pdf_is_proportional) {
            r_l = r_u / bsdf->pdf(si->wo, bsdf_sample->wi);
        }
        else {
            r_l = r_u / bsdf_sample->pdf;
        }
        specular_bounce = bsdf_sample->scatter_type.specular;
        any_nonspecular_bounce |= !specular_bounce;
        if (bsdf_sample->scatter_type.transmission) {
            ior_scale *= bsdf_sample->ior;
        }

        ray = Ray(si->point, bsdf_sample->wi, si->get_medium(bsdf_sample->wi));

        // maybe terminate early (russian roulette)
        if (russian_roulette(weight, ior_scale, depth, sampler.sample_1d(), r_u.average())) {
            break;
        }
        depth++;
    }

    return pxs;
}

std::optional<std::pair<LightSample, float>> Renderer::sample_lights(
    Pt3 point,
    const WavelengthSample& wavelengths,
    Sampler& sampler
) {
    auto [light, sample_proba] = scene.sample_lights(point, sampler);
    Vec2 sample2 = sampler.sample_2d();
    if (!light) {
        return std::nullopt;
    }
    auto ls = light->sample(point, wavelengths, sample2);
    if (!ls || ls->spec.is_zero() || ls->pdf == 0.0f) {
        return std::nullopt;
    }
    float p_l = sample_proba * ls->pdf;

    return std::make_pair(*ls, p_l);
}

SpectrumSample Renderer::sample_direct_lighting(
    const SurfaceInteraction& si,
    const BSDF& bsdf,
    const WavelengthSample& wavelengths,
    Sampler& sampler
) {
    auto sample_opt = sample_lights(si.point, wavelengths, sampler);
    if (!sample_opt) {
        return SpectrumSample(0.0f);
    }
    auto [ls, p_l] = *sample_opt;
    
    Vec3 wo = si.wo;
    Vec3 wi = ls.wi;
    auto f = bsdf(wo, wi) * std::abs(wi.dot(si.normal));
    if (f.is_zero() || scene.occluded(si.point, ls.p_light)) {
        return SpectrumSample(0.0f);
    }
    if (ls.light_type == LightType::AREA) {
        float p_b = bsdf.pdf(wo, wi);
        float w_l = power_heuristic(1, p_l, 1, p_b);
        return ls.spec * f * (w_l / p_l);
    }
    else {
        return ls.spec * f / p_l;
    }
}

// helper for sample_direct_lighting_with_media methods
SpectrumSample ray_through_media_to_light(
    Ray light_ray,
    const LightSample& ls,
    float p_l,
    const Scene& scene,
    const WavelengthSample& wavelengths,
    const SpectrumSample& f_hat,
    float scatter_pdf,
    const SpectrumSample& r_p
) {
    SpectrumSample t_ray(0.0f);
    SpectrumSample r_l(1.0f);
    SpectrumSample r_u(1.0f);
    
    std::mt19937 rng(hash(light_ray.o) + hash(light_ray.d));
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    while (!light_ray.d.is_zero()) {
        auto si_ = scene.ray_intersect(light_ray, 0.9999f);
        if (si_ && si_->material) {
            // opaque surface between current point and light
            return SpectrumSample(0.0f);
        }
        if (light_ray.medium) {
            float t_max = si_ ? si_->t : 0.9999f;
            float u = dist(rng);
            SpectrumSample t_maj = sample_majorant(
                light_ray,
                t_max,
                u,
                rng,
                wavelengths,
                [&](Pt3 p, const MediumSample& ms, const SpectrumSample& maj_val, const SpectrumSample& t_maj) {
                    // update ray transmittance estimate at sampled point
                    SpectrumSample null_val = (maj_val - ms.absorption - ms.scattering).map([](float x) { return std::max(0.0f, x); });
                    float pdf = t_maj[0] * maj_val[0];
                    t_ray *= t_maj * null_val / pdf;
                    r_l *= t_maj * maj_val / pdf;
                    r_u *= t_maj * null_val / pdf;
                    // possibly terminate via russian roullete
                    SpectrumSample tr = t_ray / (r_l + r_u).average();
                    if (tr.max_component() < 0.05f) {
                        float q = 0.75f;
                        if (dist(rng) < q) {
                            t_ray = SpectrumSample(0.0f);
                        }
                        else {
                            t_ray /= 1.0f - q;
                        }
                    }
                    return true;
                }
            );
            t_ray *= t_maj / t_maj[0];
            r_l *= t_maj / t_maj[0];
            r_u *= t_maj / t_maj[0];
        }
        if (t_ray.is_zero()) {
            return SpectrumSample(0.0f);
        }
        if (!si_) {
            break;
        }
        Vec3 rd = ls.p_light - si_->point;
        light_ray = Ray(si_->point, rd, si_->get_medium(rd));
    }

    r_l *= r_p * p_l;
    r_u *= r_p * scatter_pdf;
    if (ls.light_type == LightType::AREA) {
        return f_hat * t_ray * ls.spec / (r_l + r_u).average();
    }
    else {
        return f_hat * t_ray * ls.spec / r_l.average();
    }
}

SpectrumSample Renderer::sample_direct_lighting_with_media(
    const SurfaceInteraction& si,
    const BSDF& bsdf,
    const WavelengthSample& wavelengths,
    const SpectrumSample& r_p,
    Sampler& sampler
) {
    auto sample_opt = sample_lights(si.point, wavelengths, sampler);
    if (!sample_opt) {
        return SpectrumSample(0.0f);
    }
    auto [ls, p_l] = *sample_opt;

    Vec3 wo = si.wo;
    Vec3 wi = ls.wi;
    auto f_hat = bsdf(wo, wi) * std::abs(wi.dot(si.normal));
    float scatter_pdf = bsdf.pdf(wo, wi);

    if (f_hat.is_zero()) {
        return SpectrumSample(0.0f);
    }
    Vec3 rd = ls.p_light - si.point;
    Ray light_ray(si.point, rd, si.get_medium(rd));

    return ray_through_media_to_light(
        light_ray,
        ls,
        p_l,
        scene,
        wavelengths,
        f_hat,
        scatter_pdf,
        r_p
    );
}

SpectrumSample Renderer::sample_direct_lighting_with_media(
    const MediumInteraction& mi,
    const WavelengthSample& wavelengths,
    const SpectrumSample& r_p,
    Sampler& sampler
) {
    auto sample_opt = sample_lights(mi.point, wavelengths, sampler);
    if (!sample_opt) {
        return SpectrumSample(0.0f);
    }
    auto [ls, p_l] = *sample_opt;

    float scatter_pdf = mi.phase(mi.wo, ls.wi);
    SpectrumSample f_hat(scatter_pdf);

    if (f_hat.is_zero()) {
        return SpectrumSample(0.0f);
    }
    Vec3 rd = ls.p_light - mi.point;
    Ray light_ray(mi.point, rd, mi.medium);

    return ray_through_media_to_light(
        light_ray,
        ls,
        p_l,
        scene,
        wavelengths,
        f_hat,
        scatter_pdf,
        r_p
    );
}

void ProgressBar::display() {
    std::cout << "[";
    size_t pos = width * current / total;
    for (size_t i = 0; i < width; i++) {
        if (i < pos) {
            std::cout << "=";
        }
        else if (i == pos) {
            std::cout << ">";
        }
        else {
            std::cout << " ";
        }
    }
    std::cout << "] " << int(100.0 * current / total) << "%\r";
    std::cout.flush();
}

void ProgressBar::increment(int n, bool display) {
    std::lock_guard<std::mutex> lock(mutex);
    current += n;
    if (display) {
        this->display();
    }
}

RenderResult render(
    const Camera& camera,
    const Scene& scene,
    size_t n_samples,
    size_t max_bounces
) {
    return Renderer(camera, scene, n_samples, max_bounces).render();
}
