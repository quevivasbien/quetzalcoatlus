#include <algorithm>
#include <iostream>
#include <thread>

#include "render.hpp"

const size_t THREAD_JOB_SIZE = 4096;

Vec3 pixel_color(
    const Ray& r,
    const Primitive& world,
    size_t max_bounces,
    Sampler& sampler
) {
    Ray current_ray = r;
    Vec3 color(1.0f, 1.0f, 1.0f);

    for (size_t i = 0; i < max_bounces; i++) {
        auto isect = world.intersect(current_ray, Interval(0.001f, FLT_MAX));
        if (isect) {
            auto se = isect->material.scatter(current_ray, *isect, sampler);
            if (se) {
                color *= se->color;
                if (se->new_ray) {
                    current_ray = *(se->new_ray);
                }
                else {
                    return color;
                }
            }
            else {
                return Vec3(0.0f, 0.0f, 0.0f);
            }
        }
        else {
            // // Show sky color
            // Vec3 unit_direction = current_ray.d.normalize();
            // float t = 0.5f * (unit_direction.y + 1.0f);
            // Vec3 c = (1.0f - t) * Vec3(1.0f, 1.0f, 1.0f) + t * Vec3(0.5f, 0.7f, 1.0f);
            // return color * c;
            return Vec3(0.0f, 0.0f, 0.0f);
        }
    }

    return Vec3(0.0f, 0.0f, 0.0f);
}

void render_pixels(
    const Camera& camera,
    const Primitive* world,
    size_t n_samples,
    size_t max_bounces,
    float gamma,
    std::vector<float>& buffer,
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

        Vec3 color(0., 0., 0.);
        for (size_t s = 0; s < n_samples; s++) {
            sampler.set_sample_index(s);
            auto jitter = sampler.sample_2d();
            float u = float(x) + jitter.x;
            float v = float(y) + jitter.y;
            Ray r = camera.cast_ray(u, v);
            color += pixel_color(r, *world, max_bounces, sampler);
        }
        color /= float(n_samples);
        color.map([gamma](float c) { return powf(c, 1.0f / gamma); });

        // put values in backward, since opencv uses BGR color format
        buffer[i * 3 + 0] = color.z;
        buffer[i * 3 + 1] = color.y;
        buffer[i * 3 + 2] = color.x;
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
        gamma,
        buffer,
        start_index,
        end_index,
        global_index,
        mutex
    );
}

RenderResult render(
    const Camera& camera,
    const Primitive& world,
    size_t n_samples,
    size_t max_bounces,
    float gamma
) {
    std::vector<float> pixel_buffer(camera.image_width * camera.image_height * 3);
    std::vector<std::thread> threads;
    size_t image_size = camera.image_width * camera.image_height;
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
            &world,
            n_samples,
            max_bounces,
            gamma,
            std::ref(pixel_buffer),
            start_index,
            end_index,
            std::ref(end_index),
            std::ref(mutex)
        ));
    }
    for (auto& t : threads) {
        t.join();
    }

    return RenderResult(std::move(pixel_buffer), camera.image_height, camera.image_width);
}