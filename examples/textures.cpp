#include <iostream>

#include "camera.hpp"
#include "image.hpp"
#include "render.hpp"
#include "scene.hpp"
#include "vec.hpp"

int main() {
    Scene scene(initialize_device());

    scene.add_light(
        std::make_unique<PointLight>(
            Pt3(0., 5., 5.),
            std::make_shared<RGBIlluminantSpectrum>(RGB(8., 2., 4.)),
            60.0f
        )
    );

    // demonstrate dummy texture
    DiffuseMaterial dummy(DummyTexture{});
    scene.add_sphere(
        Pt3(-1.25, 0., -5.), 1.,
        &dummy
    );
    scene.add_grid(
        Image(
            {
                -2, -2, -3,
                0, -2.8, -3,
                2, -2, -3,

                -2, -2, -5,
                0, -2.6, -5,
                2, -2, -5,

                -2, -2, -7,
                0, -2, -7,
                2, -2, -7
            },
            3, 3
        ),
        &dummy
    );

    // demonstrate image texture
    Image image(800, 800);
    for (int y = 0; y < image.height; ++y) {
        for (int x = 0; x < image.width; ++x) {
            int index = y * image.width + x;
            image.color_buffer[3 * index + 0] = powf((float(y) / image.height - 0.5f) * 2.f, 2.f);
            image.color_buffer[3 * index + 1] = powf((float(x) / image.width - 0.5f) * 2.f, 2.f);
            image.color_buffer[3 * index + 2] = 1.f;
        }
    }
    DiffuseMaterial image_material(ImageTexture(std::move(image)));
    scene.add_sphere(
        Pt3(1.25, 0., -5.), 1.,
        &image_material
    );
    scene.add_quad(
        Pt3(-2.f, -2.f, -7.f),
        Pt3(2.f, -2.f, -7.f),
        Pt3(2.f, 2.f, -7.f),
        Pt3(-2.f, 2.f, -7.f),
        &image_material
    );

    scene.commit();

    Camera camera(
        800, 800, M_PI / 3.0f
    );
    size_t n_samples = 36;
    size_t max_bounces = 32;

    std::cout << "Rendering " << camera.image_height * camera.image_width << " pixels with " <<
        n_samples << " samples and " << max_bounces << " bounces" << std::endl;

    auto result = render(
        camera,
        scene,
        n_samples, max_bounces
    );

    result.save("textures_demo_no_denoise.png");
    result.save_albedo("textures_demo_albedo.png");
    result.save_normal("textures_demo_normal.png");

    result.denoise();
    result.save("textures_demo.png");

    return 0;
}