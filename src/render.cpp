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
// if defined, test for more than 1 intersection at a time
// NOTE: I've currently only been able to get a packet size of 4 to work, though 8 should work hypothetically
// #define PACKET_SIZE 4

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
        sample.color += weight * isect->emission(-ray.d, wavelengths);
        if (depth == max_bounces) {
            break;
        }
        auto bsdf = isect->bsdf(ray, wavelengths, sampler);
        if (!bsdf) {
            ray = isect->skip_intersection(ray);
            continue;
        }
        // TODO: add light sampling

        auto bsdf_sample = bsdf->sample(isect->wo, sampler);
        if (!bsdf_sample) {
            break;
        }
        if (depth == 0) {
            sample.albedo = sample.color + bsdf_sample->spec;
        }
        weight *= bsdf_sample->spec * std::abs(bsdf_sample->wi.dot(isect->normal)) / bsdf_sample->pdf;
        ray = Ray(isect->point, bsdf_sample->wi);
        depth++;
    }

    return sample;
}

// SpectrumSample sample_pixel_inner(
//     const Ray& r,
//     const Scene& scene,
//     const WavelengthSample& wavelengths,
//     Sampler& sampler,
//     size_t bounces
// ) {
//     if (bounces == 0) {
//         return SpectrumSample(1.0);
//     }
//     auto isect = scene.ray_intersect(r, wavelengths, sampler);
//     // no intersection, return background color
//     if (!isect) {
//         return SpectrumSample(0.0f);
//     }
//     // intersection, but no new ray; stop here
//     if (!isect->new_ray || isect->pdf == 0.f) {
//         return isect->color;
//     }
    
//     return (
//         isect->color * isect->pdf * sample_pixel_inner(*isect->new_ray, scene, wavelengths, sampler, bounces-1)
//     ) / isect->pdf;
// }

// PixelSample sample_pixel(
//     const Ray& r,
//     const Scene& scene,
//     const WavelengthSample& wavelengths,
//     Sampler& sampler,
//     size_t bounces
// ) {
//     auto isect = scene.ray_intersect(r, wavelengths, sampler);
//     if (!isect) {
//         return PixelSample();
//     }
//     PixelSample pixel_sample(isect->normal, isect->color);
//     if (!isect->new_ray || isect->pdf == 0.f) {
//         pixel_sample.color = isect->color;
//         return pixel_sample;
//     }

//     pixel_sample.color = (
//         isect->color * isect->pdf * sample_pixel_inner(*isect->new_ray, scene, wavelengths, sampler, bounces-1)
//     ) / isect->pdf;

//     return pixel_sample;
// }

// template <size_t N>
// std::array<SpectrumSample, N> sample_pixel_inner(
//     const std::array<Ray, N>& rays,
//     const std::array<int, N>& valid,
//     const Scene& scene,
//     const std::array<WavelengthSample, N>& wavelengths,
//     Sampler& sampler,
//     size_t bounces
// ) {
//     std::array<SpectrumSample, N> samples {};
//     if (bounces == 0) {
//         return samples;
//     }
//     auto isects = scene.ray_intersect(rays, wavelengths, sampler, valid);
//     std::array<Ray, N> new_rays;
//     std::array<int, N> new_valid{};
//     for (size_t i = 0; i < N; i++) {
//         if (valid[i] == 0 || !isects[i]) {
//             continue;
//         }
//         if (!isects[i]->new_ray || isects[i]->pdf == 0.f) {
//             samples[i] = isects[i]->color;
//             continue;
//         }
//         new_rays[i] = *isects[i]->new_ray;
//         new_valid[i] = -1;
//     }
    
//     int n_valid = std::accumulate(new_valid.begin(), new_valid.end(), 0);
//     if (n_valid == 0) {
//         return samples;
//     }

//     std::array<SpectrumSample, N> inner_samples;
//     // there's just one active ray left, switch to single sampling, otherwise recurse deeper
//     if (n_valid == -1) {
//         for (size_t i = 0; i < N; i++) {
//             if (valid[i] == 0) {
//                 inner_samples[i] = SpectrumSample(0.0f);
//             }
//             else {
//                 inner_samples[i] = sample_pixel_inner(
//                     rays[i],
//                     scene,
//                     wavelengths[i],
//                     sampler,
//                     bounces-1
//                 );
//             }
//         }
//     }
//     else {
//         inner_samples = sample_pixel_inner(
//             new_rays, new_valid,
//             scene,
//             wavelengths,
//             sampler,
//             bounces-1
//         );
//     }

//     for (size_t i = 0; i < N; i++) {
//         if (new_valid[i] == 0) {
//             continue;
//         }
//         samples[i] = (
//             isects[i]->color * isects[i]->pdf * inner_samples[i]
//         ) / isects[i]->pdf;
//     }
//     return samples;
// }

// template <size_t N>
// std::array<PixelSample, N> sample_pixel(
//     const std::array<Ray, N>& rays,
//     const Scene& scene,
//     const std::array<WavelengthSample, N>& wavelengths,
//     Sampler& sampler,
//     size_t bounces
// ) {
//     std::array<PixelSample, N> samples{};
//     // valid = -1 means we want to continue tracing this ray
//     // valid = 0 means we want to stop tracing this ray
//     // this is just based on embree syntax for rtcIntersect4/8/16
//     std::array<Ray, N> new_rays;
//     std::array<int, N> valid{};
//     auto isects = scene.ray_intersect(rays, wavelengths, sampler);
//     for (size_t i = 0; i < N; i++) {
//         if (!isects[i]) {
//             continue;
//         }
//         samples[i].normal = isects[i]->normal;
//         samples[i].albedo = isects[i]->color;
//         if (!isects[i]->new_ray || isects[i]->pdf == 0.f) {
//             samples[i].color = isects[i]->color;
//             continue;
//         }
//         new_rays[i] = *isects[i]->new_ray;
//         valid[i] = -1;
//     }
//     bool exit = true;
//     for (size_t i = 0; i < N; i++) {
//         if (valid[i] != 0) {
//             exit = false;
//             break;
//         }
//     }
//     if (exit) {
//         return samples;
//     }
    
//     auto inner_samples = sample_pixel_inner(
//         new_rays, valid,
//         scene,
//         wavelengths,
//         sampler,
//         bounces-1
//     );
//     for (size_t i = 0; i < N; i++) {
//         if (valid[i] == 0) {
//             continue;
//         }
//         samples[i].color = (
//             isects[i]->color * isects[i]->pdf * inner_samples[i]
//         ) / isects[i]->pdf;
//     }

//     return samples;
// }

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
        #ifdef PACKET_SIZE
        size_t n_chunks = n_samples / PACKET_SIZE;
        size_t rem  = n_samples % PACKET_SIZE;
        for (size_t chunk = 0; chunk < n_chunks; chunk++) {
            std::array<Ray, PACKET_SIZE> rays;
            std::array<WavelengthSample, PACKET_SIZE> wavelengths;
            for (size_t ss = 0; ss < PACKET_SIZE; ss++) {
                sampler.start_pixel_sample(x, y, chunk * PACKET_SIZE + ss);
                auto jitter = sampler.sample_pixel();
                float u = float(x) + jitter.x;
                float v = float(y) + jitter.y;
                rays[ss] = camera.cast_ray(u, v);
                float w = sampler.sample_1d();
                wavelengths[ss] = WavelengthSample::uniform(w);
            }
            auto samples = sample_pixel(rays, scene, wavelengths, sampler, max_bounces);
            for (size_t ss = 0; ss < PACKET_SIZE; ss++) {
                color += camera.sensor.to_sensor_rgb(samples[ss].color, wavelengths[ss]);
                albedo = camera.sensor.to_sensor_rgb(samples[ss].albedo, wavelengths[ss]);
                normal += samples[ss].normal;
            }
        }
        for (size_t s = 0; s < rem; s++) {
            sampler.start_pixel_sample(x, y, n_chunks * 4 + s);
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
        #else
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
        #endif
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
