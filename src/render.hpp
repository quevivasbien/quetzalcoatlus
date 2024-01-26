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

    RenderResult& render();

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

    PixelSample sample_pixel_with_media(
        Ray r,
        WavelengthSample& wavelengths,
        Sampler& sampler
    );

    // randomly select a light from the scene
    // returns information about the light, along with the probability of sampling that light
    std::optional<std::pair<LightSample, float>> sample_lights(
        Pt3 point,
        const WavelengthSample& wavelengths,
        Sampler& sampler
    );

    // sample direct lighting at the site of a surface interaction, ignoring any media
    SpectrumSample sample_direct_lighting(
        const SurfaceInteraction& si,
        const BSDF& bsdf,
        const WavelengthSample& wavelengths,
        Sampler& sampler
    );

    // sample direct lighting at the site of a surface interaction, accounting for light interactions in media
    SpectrumSample sample_direct_lighting_with_media(
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
        auto f_hat = bsdf(wo, wi) * std::abs(wi.dot(si.normal));
        if (f_hat.is_zero()) {
            return SpectrumSample(0.0f);
        }
        Vec3 rd = (ls.p_light - si.point).normalized();
        Ray light_ray(si.point, rd, si.get_medium(rd));

        SpectrumSample t_ray(0.0f);
        SpectrumSample r_l(1.0f);
        SpectrumSample r_u(1.0f);
        
        while (!light_ray.d.is_zero()) {
            
        }

        // todo!
        return SpectrumSample(0.0f);
    }


    // sample direct lighting at the site of a medium interaction
    SpectrumSample sample_direct_lighting_with_media(
        const MediumInteraction& mi,
        const WavelengthSample& wavelengths,
        Sampler& sampler
    ) {
        auto sample_opt = sample_lights(mi.point, wavelengths, sampler);
        if (!sample_opt) {
            return SpectrumSample(0.0f);
        }
        auto [ls, p_l] = *sample_opt;

        // todo!
        return SpectrumSample(0.0f);
    }

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