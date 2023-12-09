#include <iostream>
#include <chrono>

#include "camera.hpp"
#include "image.hpp"
#include "render.hpp"
#include "scene.hpp"
#include "vec.hpp"


int main() {
    Scene scene(initialize_device());

    auto light_color = SolidColor(
        std::make_shared<RGBIlluminantSpectrum>(RGB(8.0f, 8.0f, 8.0f))
    );
    EmissiveMaterial light{ light_color };
    scene.add_quad(
        Pt3(-1.f, 1.9999f, -6.f),
        Pt3(1.f, 1.9999f, -6.f),
        Pt3(1.f, 1.9999f, -4.f),
        Pt3(-1.f, 1.9999f, -4.f),
        &light
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
    RefractiveMaterial refractive_sphere(SolidColor(0.9f, 0.9f, 0.9f), 1.4f);
    scene.add_sphere(
        Pt3(-0.8f, -1.25f, -4.4f), 0.75f,
        &refractive_sphere
    );
    SpecularMaterial specular_sphere(SolidColor(0.96, 0.9, 1.0), 0.05);
    scene.add_sphere(
        Pt3(0.6f, -1.0f, -5.5f), 1.0f,
        &specular_sphere
    );
    
    scene.commit();

    Camera camera(
        800, 800, M_PI / 3.0f
    );
    size_t n_samples = 36;
    size_t max_bounces = 32;

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