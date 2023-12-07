#include <algorithm>
#include <array>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

#include "color/color.hpp"
#include "render.hpp"

#define MULTITHREADED
// #define PACKET_SIZE 4

const size_t THREAD_JOB_SIZE = 4096;

struct PixelSample {
    SpectrumSample color;
    Vec3 normal;
    SpectrumSample albedo;

    explicit PixelSample(const WavelengthSample& wavelengths) : color(0.0f, wavelengths), normal(0.0f, 0.0f, 0.0f), albedo(0.0f, wavelengths) {}
    PixelSample(
        const WavelengthSample& wavelengths,
        const Vec3& normal,
        const SpectrumSample& albedo
    ) : color(0.0f, wavelengths), normal(normal), albedo(albedo) {}

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

SpectrumSample sample_pixel_inner(
    const Ray& r,
    const Scene& scene,
    const WavelengthSample& wavelengths,
    Sampler& sampler,
    size_t bounces
) {
    if (bounces == 0) {
        return SpectrumSample(0.0f, wavelengths);
    }
    auto isect = scene.ray_intersect(r, wavelengths, sampler);
    if (!isect) {
        return SpectrumSample(0.0f, wavelengths);
    }
    if (!isect->new_ray || isect->pdf == 0.f) {
        return isect->color;
    }
    
    return (
        isect->color * isect->pdf * sample_pixel_inner(*isect->new_ray, scene, wavelengths, sampler, bounces-1)
    ) / isect->pdf;
}

PixelSample sample_pixel(
    const Ray& r,
    const Scene& scene,
    const WavelengthSample& wavelengths,
    Sampler& sampler,
    size_t bounces
) {
    auto isect = scene.ray_intersect(r, wavelengths, sampler);
    if (!isect) {
        return PixelSample(wavelengths);
    }
    PixelSample pixel_sample(wavelengths, isect->normal, isect->color);
    if (!isect->new_ray || isect->pdf == 0.f) {
        pixel_sample.color = isect->color;
        return pixel_sample;
    }

    pixel_sample.color = (
        isect->color * isect->pdf * sample_pixel_inner(*isect->new_ray, scene, wavelengths, sampler, bounces-1)
    ) / isect->pdf;

    return pixel_sample;
}

template <size_t N>
std::vector<SpectrumSample> sample_pixel_inner(
    const std::array<Ray, N>& rays,
    const std::array<int, N>& valid,
    const Scene& scene,
    const std::vector<WavelengthSample>& wavelengths,
    Sampler& sampler,
    size_t bounces
) {
    std::vector<SpectrumSample> samples;
    samples.reserve(N);
    for (size_t i = 0; i < N; i++) {
        samples.push_back(SpectrumSample(0.0f, wavelengths[i]));
    }
    if (bounces == 0) {
        return samples;
    }
    auto isects = scene.ray_intersect(rays, wavelengths, sampler, valid);
    std::array<Ray, N> new_rays;
    std::array<int, N> new_valid{};
    for (size_t i = 0; i < N; i++) {
        if (valid[i] == 0 || !isects[i]) {
            continue;
        }
        if (!isects[i]->new_ray || isects[i]->pdf == 0.f) {
            samples[i] = isects[i]->color;
            continue;
        }
        new_rays[i] = *isects[i]->new_ray;
        new_valid[i] = -1;
    }
    bool exit = true;
    for (size_t i = 0; i < N; i++) {
        if (new_valid[i] != 0) {
            exit = false;
            break;
        }
    }
    if (exit) {
        return samples;
    }
    auto inner_samples = sample_pixel_inner(
        new_rays, new_valid,
        scene,
        wavelengths,
        sampler,
        bounces-1
    );
    for (size_t i = 0; i < N; i++) {
        if (new_valid[i] == 0) {
            continue;
        }
        samples[i] = (
            isects[i]->color * isects[i]->pdf * inner_samples[i]
        ) / isects[i]->pdf;
    }
    return samples;
}

template <size_t N>
std::vector<PixelSample> sample_pixel(
    const std::array<Ray, N>& rays,
    const Scene& scene,
    const std::vector<WavelengthSample>& wavelengths,
    Sampler& sampler,
    size_t bounces
) {
    std::vector<PixelSample> samples;
    samples.reserve(N);
    for (size_t i = 0; i < N; i++) {
        samples.push_back(PixelSample(wavelengths[i]));
    }
    // valid = -1 means we want to continue tracing this ray
    // valid = 0 means we want to stop tracing this ray
    // this is just based on embree syntax for rtcIntersect4/8/16
    std::array<Ray, N> new_rays;
    std::array<int, N> valid{};
    auto isects = scene.ray_intersect(rays, wavelengths, sampler);
    for (size_t i = 0; i < N; i++) {
        if (!isects[i]) {
            continue;
        }
        samples[i].normal = isects[i]->normal;
        samples[i].albedo = isects[i]->color;
        if (!isects[i]->new_ray || isects[i]->pdf == 0.f) {
            samples[i].color = isects[i]->color;
            continue;
        }
        new_rays[i] = *isects[i]->new_ray;
        valid[i] = -1;
    }
    bool exit = true;
    for (size_t i = 0; i < N; i++) {
        if (valid[i] != 0) {
            exit = false;
            break;
        }
    }
    if (exit) {
        return samples;
    }
    
    auto inner_samples = sample_pixel_inner(
        new_rays, valid,
        scene,
        wavelengths,
        sampler,
        bounces-1
    );
    for (size_t i = 0; i < N; i++) {
        if (valid[i] == 0) {
            continue;
        }
        samples[i].color = (
            isects[i]->color * isects[i]->pdf * inner_samples[i]
        ) / isects[i]->pdf;
    }

    return samples;
}

void render_pixels(
    const Camera& camera,
    const Scene& scene,
    size_t n_samples,
    size_t max_bounces,
    RenderResult& result,
    size_t start_index,
    size_t end_index,
    size_t& global_index,
    std::mutex& mutex
) {
    uint32_t stratum_width = sqrtl(n_samples);
    for (size_t i = start_index; i < end_index; i++) {
        Sampler sampler(
            static_cast<uint32_t>(i),
            stratum_width
        );
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
            std::vector<WavelengthSample> wavelengths;
            wavelengths.reserve(PACKET_SIZE);
            for (size_t ss = 0; ss < PACKET_SIZE; ss++) {
                sampler.set_sample_index(chunk * PACKET_SIZE + ss);
                auto jitter = sampler.sample_2d();
                float u = float(x) + jitter.x;
                float v = float(y) + jitter.y;
                rays[ss] = camera.cast_ray(u, v);
                float w = sampler.sample_1d();
                wavelengths.push_back(WavelengthSample::uniform(w));
            }
            auto samples = sample_pixel(rays, scene, wavelengths, sampler, max_bounces);
            for (size_t s = 0; s < PACKET_SIZE; s++) {
                color += PixelSensor::CIE_XYZ().to_sensor_rgb(samples[s].color);
                albedo = PixelSensor::CIE_XYZ().to_sensor_rgb(samples[s].albedo);
                normal += samples[s].normal;
            }
        }
        for (size_t s = 0; s < rem; s++) {
            sampler.set_sample_index(n_chunks * 4 + s);
            auto jitter = sampler.sample_2d();
            float u = float(x) + jitter.x;
            float v = float(y) + jitter.y;
            Ray r = camera.cast_ray(u, v);
            WavelengthSample wavelengths = WavelengthSample::uniform(sampler.sample_1d());
            auto sample = sample_pixel(r, scene, wavelengths, sampler, max_bounces);
            color += PixelSensor::CIE_XYZ().to_sensor_rgb(sample.color);
            albedo += PixelSensor::CIE_XYZ().to_sensor_rgb(sample.albedo);
            normal += sample.normal;
        }
        #else
        for (size_t s = 0; s < n_samples; s++) {
            sampler.set_sample_index(s);
            auto jitter = sampler.sample_2d();
            float u = float(x) + jitter.x;
            float v = float(y) + jitter.y;
            Ray r = camera.cast_ray(u, v);
            WavelengthSample wavelengths = WavelengthSample::uniform(sampler.sample_1d());
            auto sample = sample_pixel(r, scene, wavelengths, sampler, max_bounces);
            color += PixelSensor::CIE_XYZ().to_sensor_rgb(sample.color);
            albedo += PixelSensor::CIE_XYZ().to_sensor_rgb(sample.albedo);
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
        n_samples,
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
    
    std::vector<std::thread> threads;
    size_t image_size = result.width * result.height;
    #ifdef MULTITHREADED
    size_t n_threads = std::clamp<size_t>(
        std::thread::hardware_concurrency(),
        1, (image_size + THREAD_JOB_SIZE - 1) / THREAD_JOB_SIZE
    );
    #else
    size_t n_threads = 1;
    #endif
    std::cout << "Using " << n_threads << " threads" << std::endl;
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
            camera,
            std::ref(scene),
            n_samples,
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

    return result;
}
