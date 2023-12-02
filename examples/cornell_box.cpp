#include <iostream>
#include <chrono>

#include "camera.hpp"
#include "image.hpp"
#include "render.hpp"
#include "scene.hpp"
#include "vec.hpp"


int main() {
    // ShapePrimitive light(
    //     Quad(Pt3(-1.0, 1.99, -4.0), Vec3(0.0, 0.0, -2.0), Vec3(2.0, 0.0, 0.0)),
    //     EmissiveMaterial(SolidColor(4.0, 4.0, 4.0))
    // );
    // ShapePrimitive ceiling(
    //     Quad(Pt3(-2.0, 2.0, -3.0), Vec3(0.0, 0.0, -4.0), Vec3(4.0, 0.0, 0.0)),
    //     LambertMaterial(SolidColor(0.8f, 0.8f, 0.2f))
    // );
    // ShapePrimitive floor(
    //     Quad(Pt3(-2.0, -2.0, -3.0), Vec3(4.0, 0.0, 0.0), Vec3(0.0, 0.0, -4.0)),
    //     LambertMaterial(SolidColor(0.3f, 0.8f, 0.8f))
    // );
    // ShapePrimitive left_wall(
    //     Quad(Pt3(-2.0, -2.0, -3.0), Vec3(0.0, 0.0, -4.0), Vec3(0.0, 4.0, 0.0)),
    //     LambertMaterial(SolidColor(1.0f, 0.2f, 0.2f))
    // );
    // ShapePrimitive right_wall(
    //     Quad(Pt3(2.0, -2.0, -3.0), Vec3(0.0, 4.0, 0.0), Vec3(0.0, 0.0, -4.0)),
    //     LambertMaterial(SolidColor(0.3f, 0.3f, 0.8f))
    // );
    // ShapePrimitive back_wall(
    //     Quad(Pt3(-2.0, -2.0, -7.0), Vec3(4.0, 0.0, 0.0), Vec3(0.0, 4.0, 0.0)),
    //     LambertMaterial(SolidColor(0.2f, 0.8f, 0.2f))
    // );

    // ShapePrimitive specular_sphere(
    //     Sphere(Pt3(0.0f, -1.0f, -5.0f), 1.0),
    //     SpecularMaterial(SolidColor(0.3f, 0.6f, 0.8f), 0.1f)
    // );

    // Aggregate world({&light, &ceiling, &floor, &left_wall, &right_wall, &back_wall, &specular_sphere});

    Scene scene(initialize_device());

    EmissiveMaterial light(SolidColor(4.0f, 4.0f, 4.0f));
    scene.add_quad(
        Pt3(-1.f, 1.9999f, -6.f),
        Pt3(1.f, 1.9999f, -6.f),
        Pt3(1.f, 1.9999f, -4.f),
        Pt3(-1.f, 1.9999f, -4.f),
        &light
    );
    LambertMaterial ceiling(SolidColor(0.8f, 0.8f, 0.2f));
    scene.add_quad(
        Pt3(-2.f, 2.f, -7.f),
        Pt3(2.f, 2.f, -7.f),
        Pt3(2.f, 2.f, -3.f),
        Pt3(-2.f, 2.f, -3.f),
        &ceiling
    );
    LambertMaterial floor(SolidColor(0.3f, 0.8f, 0.8f));
    scene.add_quad(
        Pt3(-2.f, -2.f, -3.f),
        Pt3(2.f, -2.f, -3.f),
        Pt3(2.f, -2.f, -7.f),
        Pt3(-2.f, -2.f, -7.f),
        &floor
    );
    LambertMaterial left_wall(SolidColor(1.0f, 0.2f, 0.2f));
    scene.add_quad(
        Pt3(-2.f, -2.f, -3.f),
        Pt3(-2.f, -2.f, -7.f),
        Pt3(-2.f, 2.f, -7.f),
        Pt3(-2.f, 2.f, -3.f),
        &left_wall
    );
    LambertMaterial right_wall(SolidColor(0.3f, 0.3f, 0.8f));
    scene.add_quad(
        Pt3(2.f, -2.f, -3.f),
        Pt3(2.f, 2.f, -3.f),
        Pt3(2.f, 2.f, -7.f),
        Pt3(2.f, -2.f, -7.f),
        &right_wall
    );
    LambertMaterial back_wall(SolidColor(0.2f, 0.8f, 0.2f));
    scene.add_quad(
        Pt3(-2.f, -2.f, -7.f),
        Pt3(2.f, -2.f, -7.f),
        Pt3(2.f, 2.f, -7.f),
        Pt3(-2.f, 2.f, -7.f),
        &back_wall
    );
    SpecularMaterial sphere(SolidColor(0.3f, 0.6f, 0.8f), 0.1f);
    scene.add_sphere(
        Pt3(0.0f, -1.0f, -5.0f), 1.0f,
        &sphere
    );
    
    scene.commit();

    Camera camera(
        800, 800, M_PI / 3.0f
    );
    size_t n_samples = 16;
    size_t max_bounces = 16;

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