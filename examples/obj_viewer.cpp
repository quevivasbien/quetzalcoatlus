#include <iostream>
#include <string>
#include "render.hpp"

int main(int argc, const char* const argv[]) {
    std::string filename;
    Vec3 camera_position(0., 2.0, 7);
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
            std::cerr << "No camera position provided, using (0, 2, 7)" << std::endl;
        }
    }

    // check that filename ends with .obj
    if (filename.substr(filename.length() - 4) != ".obj") {
        std::cerr << "File name must end with .obj" << std::endl;
        return 1;
    }

    Scene scene(initialize_device());
    EmissiveMaterial light(SolidColor(1.0, 1.0, 1.0));
    scene.add_plane(
        Pt3(0., 100., 100.),
        Vec3(0., -1., -1.).normalize(),
        &light
    );
    LambertMaterial material(SolidColor(0.8, 0.8, 0.8));
    scene.add_obj(filename, &material);
    scene.commit();

    Camera camera(
        1920, 1080, M_PI / 3.0,
        Transform::translation(camera_position)
    );
    size_t n_samples = 16;
    size_t max_bounces = 64;

    auto result = render(camera, scene, n_samples, max_bounces);
    result.denoise();

    // get filename by removing .obj and adding .png
    std::string filename_png = filename.substr(0, filename.length() - 4) + ".png";
    result.save(filename_png);

    return 0;
}