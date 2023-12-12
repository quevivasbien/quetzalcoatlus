#include <iostream>
#include <memory>
#include <string>
#include "render.hpp"

int main(int argc, const char* const argv[]) {
    std::string filename;
    Vec3 camera_position(0., 2.0, 6);
    Vec3 camera_rotation(0., 0., 0.);
    if (argc < 2) {
        std::cerr << "Usage: program_name file_name [camera_x camera_y camera_z] [rotate_x rotate_y rotate_z]" << std::endl;
        std::cerr << "No file provided, trying with file_name = teapot.obj" << std::endl;
        filename = "teapot.obj";
    }
    else {
        filename = argv[1];
        if (argc >= 4) {
            camera_position = Vec3(std::stof(argv[2]), std::stof(argv[3]), std::stof(argv[4]));
        }
        else {
            std::cerr << "No camera position provided, using (0, 2, 6)" << std::endl;
        }
        if (argc >= 7) {
            camera_rotation = Vec3(std::stof(argv[5]), std::stof(argv[6]), std::stof(argv[7]));
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

    auto light_spectrum = std::make_shared<RGBIlluminantSpectrum>(RGB(3.0, 2.0, 3.0));
    scene.add_light(std::make_unique<AreaLight>(
        std::make_unique<Quad>(
            Pt3(4.f, 6.f, 8.f),
            Vec3(2.f, 0.f, -2.f),
            Vec3(0.f, 3.f, -3.f)
        ),
        light_spectrum,
        8.0f
    ));

    DiffuseMaterial material(SolidColor(1.0, 6.0, 1.0));
    // DielectricMaterial material(std::make_shared<RGBUnboundedSpectrum>(1.2, 1.4, 3.0));
    scene.add_obj(filename, &material);

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

    Transform camera_t =
        Transform::translation(camera_position)
        * Transform::rotate_x(camera_rotation.x)
        * Transform::rotate_y(camera_rotation.y)
        * Transform::rotate_z(camera_rotation.z);
    Camera camera(
        1920, 1080, M_PI / 3.0,
        camera_t
    );
    size_t n_samples = 16;
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