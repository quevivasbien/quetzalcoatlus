#include <iostream>
#include <memory>
#include <string>
#include "render.hpp"

const std::string DEFAULT_FILE = "teapot.obj";

int main(int argc, const char* const argv[]) {
    std::string filename;
    Vec3 position(0., 0., 0.);
    float scale = 1.0;
    Vec3 rotation(0., 0., 0.);
    if (argc < 2) {
        std::cerr << "Usage: program_name file_name [camera_x camera_y camera_z] [scale] [rotate_x rotate_y rotate_z]" << std::endl;
        std::cerr << "No file provided, trying with file_name = " << DEFAULT_FILE << std::endl;
        filename = DEFAULT_FILE;
    }
    else {
        filename = argv[1];
        if (argc >= 4) {
            position = Vec3(std::stof(argv[2]), std::stof(argv[3]), std::stof(argv[4]));
        }
        else {
            std::cerr << "No camera position provided, using (0, 0, 0)" << std::endl;
        }
        if (argc >= 5) {
            scale = std::stof(argv[5]);
        }
        else {
            std::cerr << "No camera scale provided, using 1.0" << std::endl;
        }
        if (argc >= 8) {
            rotation = Vec3(std::stof(argv[6]), std::stof(argv[7]), std::stof(argv[8]));
        }
        else {
            std::cerr << "No camera rotation provided, using (0, 0, 0)" << std::endl;
        }
    }

    // check that filename ends with .obj
    if (filename.substr(filename.length() - 4) != ".obj") {
        std::cerr << "File name must end with .obj" << std::endl;
        return 1;
    }

    Scene scene(initialize_device());

    auto light_spectrum = std::make_shared<RGBIlluminantSpectrum>(RGB(3.0, 1.0, 2.0));
    // scene.add_light(std::make_unique<AreaLight>(
    //     std::make_unique<Quad>(
    //         Pt3(4.f, 6.f, 8.f),
    //         Vec3(2.f, 0.f, -2.f),
    //         Vec3(0.f, 3.f, -3.f)
    //     ),
    //     light_spectrum,
    //     8.0f
    // ));
    scene.add_light(std::make_unique<PointLight>(
        Pt3(4., 6., 8.),
        light_spectrum,
        50.0f
    ));

    auto material = ConductiveMaterial::copper(0.12, 0.2);
    Transform transform =
        Transform::translation(position)
        * Transform::rotate_x(rotation.x)
        * Transform::rotate_y(rotation.y)
        * Transform::rotate_z(rotation.z)
        * Transform::scale(scale);
    scene.add_obj(filename, &material, transform);

    DiffuseMaterial floor(SolidColor(1.0, 0.4, 0.9));
    scene.add_plane(
        Pt3(0., -0.1, 0.),
        Vec3(0., 1., 0.),
        &floor
    );
    scene.add_plane(
        Pt3(0., 0., -5.),
        Vec3(0., 0., 1.),
        &floor
    );

    scene.commit();

    Camera camera(
        1920, 1080, M_PI / 3.0,
        Transform::translation(0., 4., 6.)
        * Transform::rotate_x(-M_PI / 8.0)
    );
    size_t n_samples = 32;
    size_t max_bounces = 64;

    auto result = render(camera, scene, n_samples, max_bounces);

    // get filename base by removing .obj
    std::string filename_base = filename.substr(0, filename.length() - 4);

    result.save_albedo(filename_base + "_albedo.png");
    result.save_normal(filename_base + "_normal.png");
    result.save(filename_base + "_no_denoise.png");

    result.denoise();
    result.save(filename_base + ".png");

    return 0;
}