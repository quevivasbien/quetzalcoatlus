#include <algorithm>
#include <iostream>
#include <mutex>
#include <thread>

#include "render.hpp"

const size_t THREAD_JOB_SIZE = 4096;

struct PixelSample {
    Vec3 color;
    Vec3 normal;
    Vec3 albedo;

    PixelSample() : color(0.0f, 0.0f, 0.0f), normal(0.0f, 0.0f, 0.0f), albedo(0.0f, 0.0f, 0.0f) {}
    
    void operator+=(const PixelSample& other) {
        color += other.color;
        normal += other.normal;
        albedo += other.albedo;
    }

    void operator/=(float c) {
        color /= c;
        normal /= c;
        albedo /= c;
    }
};

PixelSample sample_pixel(
    const Ray& r,
    const Scene& world,
    size_t max_bounces,
    Sampler& sampler
) {
    Ray current_ray = r;
    PixelSample sample;
    sample.color = Vec3(1.0f, 1.0f, 1.0f);

    for (size_t i = 0; i < max_bounces; i++) {
        auto isect = world.ray_intersect(current_ray, sampler);
        if (isect) {
            if (i == 0) {
                sample.normal = isect->normal;
                sample.albedo = isect->color;
            }
            sample.color *= isect->color;
            if (isect->new_ray) {
                current_ray = *(isect->new_ray);
            }
            else {
                return sample;
            }
        }
        else {
            sample.color = Vec3(0.0f, 0.0f, 0.0f);
            return sample;
        }
    }

    sample.color = Vec3(0.0f, 0.0f, 0.0f);
    return sample;
}

void render_pixels(
    const Camera& camera,
    const Scene& world,
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

        PixelSample pixel{};
        for (size_t s = 0; s < n_samples; s++) {
            sampler.set_sample_index(s);
            auto jitter = sampler.sample_2d();
            float u = float(x) + jitter.x;
            float v = float(y) + jitter.y;
            Ray r = camera.cast_ray(u, v);
            pixel += sample_pixel(r, world, max_bounces, sampler);
        }
        pixel /= float(n_samples);

        result.color_buffer[i * 3 + 0] = pixel.color.x;
        result.color_buffer[i * 3 + 1] = pixel.color.y;
        result.color_buffer[i * 3 + 2] = pixel.color.z;

        result.normal_buffer[i * 3 + 0] = pixel.normal.x;
        result.normal_buffer[i * 3 + 1] = pixel.normal.y;
        result.normal_buffer[i * 3 + 2] = pixel.normal.z;

        result.albedo_buffer[i * 3 + 0] = pixel.albedo.x;
        result.albedo_buffer[i * 3 + 1] = pixel.albedo.y;
        result.albedo_buffer[i * 3 + 2] = pixel.albedo.z;
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
        world,
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
    if (!scene.ready) {
        std::cout << "Scene must be committed before rendering." << std::endl;
        return result;
    }
    
    std::vector<std::thread> threads;
    size_t image_size = result.width * result.height;
    size_t n_threads = std::clamp<size_t>(
        std::thread::hardware_concurrency(),
        1, (image_size + THREAD_JOB_SIZE - 1) / THREAD_JOB_SIZE
    );
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
