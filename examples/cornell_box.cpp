#include <iostream>
#include <chrono>

#include "camera.hpp"
#include "image.hpp"
#include "render.hpp"
#include "scene.hpp"
#include "vec.hpp"


int main() {
    Scene scene(initialize_device());

    auto light_spectrum = 
        spectra::ILLUM_D65();
    auto light_shape = std::make_unique<Quad>(
        Pt3(-1.f, 1.9999f, -4.f),
        Vec3(0.f, 0.f, -2.f),
        Vec3(2.f, 0.f, 0.f)
    );
    scene.add_light(
        std::make_unique<AreaLight>(
            std::move(light_shape),
            light_spectrum,
            16.0f,
            false
        )
    );

    DiffuseMaterial ceiling(SolidColor(0.8f, 0.4f, 0.1f));
    scene.add_quad(
        Pt3(-2.f, 2.f, -7.f),
        Pt3(2.f, 2.f, -7.f),
        Pt3(2.f, 2.f, -3.f),
        Pt3(-2.f, 2.f, -3.f),
        &ceiling
    );
    DiffuseMaterial floor(SolidColor(0.1f, 0.6f, 0.8f));
    scene.add_quad(
        Pt3(-2.f, -2.f, -3.f),
        Pt3(2.f, -2.f, -3.f),
        Pt3(2.f, -2.f, -7.f),
        Pt3(-2.f, -2.f, -7.f),
        &floor
    );
    DiffuseMaterial left_wall(SolidColor(0.8f, 0.0f, 0.1f));
    scene.add_quad(
        Pt3(-2.f, -2.f, -3.f),
        Pt3(-2.f, -2.f, -7.f),
        Pt3(-2.f, 2.f, -7.f),
        Pt3(-2.f, 2.f, -3.f),
        &left_wall
    );
    DiffuseMaterial right_wall(SolidColor(0.1f, 0.1f, 0.8f));
    scene.add_quad(
        Pt3(2.f, -2.f, -3.f),
        Pt3(2.f, 2.f, -3.f),
        Pt3(2.f, 2.f, -7.f),
        Pt3(2.f, -2.f, -7.f),
        &right_wall
    );
    DiffuseMaterial back_wall(SolidColor(0.1f, 0.8f, 0.1f));
    scene.add_quad(
        Pt3(-2.f, -2.f, -7.f),
        Pt3(2.f, -2.f, -7.f),
        Pt3(2.f, 2.f, -7.f),
        Pt3(-2.f, 2.f, -7.f),
        &back_wall
    );
    DielectricMaterial dielectric(std::make_shared<RGBUnboundedSpectrum>(RGB(1.1f, 1.8f, 3.0f)));
    scene.add_sphere(
        Pt3(-0.8f, -1.25f, -4.4f), 0.75f,
        &dielectric
    );
    auto metal = ConductiveMaterial::copper();
    scene.add_sphere(
        Pt3(0.6f, -1.0f, -5.5f), 1.0f,
        &metal
    );
    
    scene.commit();

    Camera camera(
        800, 800, M_PI / 3.0f
    );
    size_t n_samples = 128;
    size_t max_bounces = 64;

    std::cout << "Rendering " << camera.image_height * camera.image_width << " pixels with " <<
        n_samples << " samples and " << max_bounces << " bounces" << std::endl;

    auto start_time = std::chrono::steady_clock::now();
    auto result = render(
        camera,
        scene,
        n_samples, max_bounces
    );
    auto end_time = std::chrono::steady_clock::now();
    std::cout << "Render time: " <<
        std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count() <<
        "ms" << std::endl;

    result.save("cornell_box_no_denoise.png");

    Image result_im(result.height, result.width);
    result_im.color_buffer = result.color_buffer;
    result_im.denoise();
    result_im.save("cornell_box_denoise.png");

    Image albedo_im(result.height, result.width);
    albedo_im.color_buffer = result.albedo_buffer;
    albedo_im.save("cornell_box_albedo.png");

    Image normal_im(result.height, result.width);
    normal_im.color_buffer = result.normal_buffer;
    normal_im.save("cornell_box_normal.png");
    
    result.denoise();
    result.save("cornell_box_advanced_denoise.png");

    return 0;
}