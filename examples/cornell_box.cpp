#include <iostream>
#include <chrono>

#include "camera.hpp"
#include "primitive.hpp"
#include "render.hpp"
#include "image.hpp"
#include "vec.hpp"


int main() {
    ShapePrimitive light(
        Quad(Pt3(-1.0, 1.99, -4.0), Vec3(0.0, 0.0, -2.0), Vec3(2.0, 0.0, 0.0)),
        EmissiveMaterial(SolidColor(4.0, 4.0, 4.0))
    );
    ShapePrimitive ceiling(
        Quad(Pt3(-2.0, 2.0, -3.0), Vec3(0.0, 0.0, -4.0), Vec3(4.0, 0.0, 0.0)),
        LambertMaterial(SolidColor(0.8f, 0.8f, 0.2f))
    );
    ShapePrimitive floor(
        Quad(Pt3(-2.0, -2.0, -3.0), Vec3(4.0, 0.0, 0.0), Vec3(0.0, 0.0, -4.0)),
        LambertMaterial(SolidColor(0.3f, 0.8f, 0.8f))
    );
    ShapePrimitive left_wall(
        Quad(Pt3(-2.0, -2.0, -3.0), Vec3(0.0, 0.0, -4.0), Vec3(0.0, 4.0, 0.0)),
        LambertMaterial(SolidColor(1.0f, 0.2f, 0.2f))
    );
    ShapePrimitive right_wall(
        Quad(Pt3(2.0, -2.0, -3.0), Vec3(0.0, 4.0, 0.0), Vec3(0.0, 0.0, -4.0)),
        LambertMaterial(SolidColor(0.3f, 0.3f, 0.8f))
    );
    ShapePrimitive back_wall(
        Quad(Pt3(-2.0, -2.0, -7.0), Vec3(4.0, 0.0, 0.0), Vec3(0.0, 4.0, 0.0)),
        LambertMaterial(SolidColor(0.2f, 0.8f, 0.2f))
    );

    ShapePrimitive specular_sphere(
        Sphere(Pt3(0.0f, -1.0f, -5.0f), 1.0),
        SpecularMaterial(SolidColor(0.3f, 0.6f, 0.8f), 0.1f)
    );

    Aggregate world({&light, &ceiling, &floor, &left_wall, &right_wall, &back_wall, &specular_sphere});

    Camera camera(
        800, 800, M_PI / 3.0f
    );
    size_t n_samples = 36;
    size_t max_bounces = 16;

    std::cout << "Rendering " << camera.image_height * camera.image_width << " pixels with " <<
        n_samples << " samples and " << max_bounces << " bounces" << std::endl;

    auto start_time = std::chrono::steady_clock::now();
    auto result = render(
        camera,
        world,
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
    
    result.denoise();
    result.save("cornell_box_pretty_denoise.png");

    return 0;
}