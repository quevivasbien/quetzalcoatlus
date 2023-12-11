#include <algorithm>
#include <array>
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

PixelSample sample_pixel(
    Ray ray,
    const Scene& scene,
    WavelengthSample& wavelengths,
    Sampler& sampler,
    size_t max_bounces
) {
    PixelSample sample{};
    SpectrumSample weight(1.0f);
    size_t depth = 0;
    while (!weight.is_zero()) {
        auto isect = scene.ray_intersect(ray, wavelengths, sampler);
        // no intersection, add background and break
        // TODO: add background / infinite lights
        if (!isect) {
            break;
        }
        if (depth == 0) {
            sample.normal = isect->normal;
        }
        // TODO: change this when specular bounce and light sampling is implemented
        // sample.color += weight * isect->emission(-ray.d, wavelengths);
        if (depth == max_bounces) {
            break;
        }
        auto bsdf = isect->bsdf(ray, wavelengths, sampler);
        if (!bsdf) {
            ray = isect->skip_intersection(ray);
            continue;
        }
        
        auto [light_sample, proba] = scene.sample_lights(*isect, wavelengths, sampler);
        if (light_sample && !light_sample->spec.is_zero() && light_sample->pdf > 0.0f) {
            auto f = (*bsdf)(isect->wo, light_sample->wi);
            if (!f.is_zero() && !scene.occluded(isect->point, light_sample->p_light)) {
                sample.color += weight * f * light_sample->spec / (proba * light_sample->pdf);
            }
        }

        auto bsdf_sample = bsdf->sample(isect->wo, sampler);
        if (!bsdf_sample) {
            break;
        }
        if (depth == 0) {
            sample.albedo = bsdf_sample->spec;
        }
        weight *= bsdf_sample->spec * std::abs(bsdf_sample->wi.dot(isect->normal)) / bsdf_sample->pdf;
        ray = Ray(isect->point, bsdf_sample->wi);
        depth++;
    }

    return sample;
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
            auto sample = sample_pixel(r, scene, wavelengths, sampler, max_bounces);
            color += camera.sensor.to_sensor_rgb(sample.color, wavelengths);
            albedo += camera.sensor.to_sensor_rgb(sample.albedo, wavelengths);
            normal += sample.normal;
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

    // check that ray packet size is compatible with machine
    #if PACKET_SIZE == 4
    if (rtcGetDeviceProperty(scene.get_device(), RTC_DEVICE_PROPERTY_NATIVE_RAY4_SUPPORTED) == 0) {
        std::cout << "Warning: packet size 4 selected but not supported" << std::endl;
    }
    #elif PACKET_SIZE == 8
    if (rtcGetDeviceProperty(scene.get_device(), RTC_DEVICE_PROPERTY_NATIVE_RAY8_SUPPORTED) == 0) {
        std::cout << "Warning: packet size 8 selected but not supported" << std::endl;
    }
    #endif
    
    size_t image_size = result.width * result.height;
    #ifdef MULTITHREADED
    std::vector<std::thread> threads;
    size_t n_threads = std::clamp<size_t>(
        std::thread::hardware_concurrency(),
        1, (image_size + THREAD_JOB_SIZE - 1) / THREAD_JOB_SIZE
    );
    std::cout << "Using " << n_threads << " threads" << std::endl;

    std::vector<HaltonSampler> samplers;
    samplers.reserve(n_threads);
    for (size_t t = 0; t < n_threads; t++) {
        samplers.push_back(HaltonSampler(n_samples, camera.image_width, camera.image_height, t));
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
    HaltonSampler sampler(n_samples, camera.image_width, camera.image_height, 0);
    std::mutex _mutex;
    render_pixels(
        camera, scene, sampler, max_bounces,
        result, 0, image_size, image_size, _mutex
    );
    #endif

    return result;
}
