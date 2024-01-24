#pragma once

#include "camera.hpp"
#include "scene.hpp"
#include "image.hpp"
#include "vec.hpp"

struct PixelSample {
    SpectrumSample color;
    Vec3 normal;
    SpectrumSample albedo;

    PixelSample() : color(0.0f), normal(0.0f, 0.0f, 0.0f), albedo(0.0f) {}
    PixelSample(
        const Vec3& normal,
        const SpectrumSample& albedo
    ) : color(0.0f), normal(normal), albedo(albedo) {}

    PixelSample& operator+=(const PixelSample& other);
    PixelSample& operator/=(float c);
};  

struct ProgressBar {
    size_t total;
    size_t current = 0;
    size_t width = 40;
    std::mutex mutex;
    
    void display();
    void increment(int n = 1, bool display = true);
};

class Renderer {
public:
    Renderer(
        const Camera& camera,
        const Scene& scene,
        size_t n_samples,
        size_t max_bounces
    );

    RenderResult render();

private:
    void render_pixels(
        size_t start_index,
        size_t end_index,
        Sampler& sampler
    );

    PixelSample sample_pixel(
        Ray r,
        WavelengthSample& wavelengths,
        Sampler& sampler
    );

    SpectrumSample sample_lights(
        const SurfaceInteraction& si,
        const BSDF& bsdf,
        const WavelengthSample& wavelengths,
        Sampler& sampler
    );

    PixelSample sample_pixel_with_media(
        Ray r,
        WavelengthSample& wavelengths,
        Sampler& sampler
    );

    // SpectrumSample sample_lights_with_media(

    // ) {
    //     //todo!
    // }

    const Camera& camera;
    const Scene& scene;
    const size_t n_samples;
    const size_t max_bounces;

    const bool use_media;

    RenderResult result;
    ProgressBar progress_bar;

    size_t current_index = 0;

    std::mutex mutex;
};


RenderResult render(
    const Camera& camera,
    const Scene& world,
    size_t n_samples,
    size_t max_bounces
);