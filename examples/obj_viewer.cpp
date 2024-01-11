#include <iostream>
#include <memory>
#include <string>
#include "render.hpp"

// just for command line options here
#include <opencv2/opencv.hpp>

const std::string DEFAULT_FILE = "teapot.obj";

int main(int argc, const char* const argv[]) {
    cv::String keys =
        "{help h usage ? | | Print this message.}"
        "{@file           | " + DEFAULT_FILE + " | Input file.}"
        "{x x_offset     | 0. | X offset.}"
        "{y y_offset     | 0. | Y offset.}"
        "{z z_offset     | 0. | Z offset.}"
        "{s scale        | 1. | Scale.}"
        "{rx x_rotation   | 0. | rotation about X axis.}"
        "{ry y_rotation   | 0. | rotation about Y axis.}"
        "{rz z_rotation   | 0. | rotation about Z axis.}"
        "{width        | 800 | Image width.}"
        "{height       | 600 | Image height.}"
        "{m material | diffuse | Material type, one of diffuse, copper, alluminum, glass}"
        ;
    cv::CommandLineParser parser(argc, argv, keys);
    if (parser.has("help")) {
        parser.printMessage();
        return 0;
    }
    std::string filename = parser.get<cv::String>(0);
    Vec3 position;
    position.x = parser.get<float>("x");
    position.y = parser.get<float>("y");
    position.z = parser.get<float>("z");
    float scale = parser.get<float>("s");
    Vec3 rotation;
    rotation.x = parser.get<float>("rx");
    rotation.y = parser.get<float>("ry");
    rotation.z = parser.get<float>("rz");
    int width = parser.get<int>("width");
    int height = parser.get<int>("height");
    std::string material_type = parser.get<std::string>("m");
    
    std::unique_ptr<Material> material;
    if (material_type == "diffuse") {
        material = std::make_unique<DiffuseMaterial>(SolidColor(0.6, 0.8, 0.8));
    }
    else if (material_type == "copper") {
        material = std::make_unique<ConductiveMaterial>(ConductiveMaterial::copper(0.12, 0.2));
    }
    else if (material_type == "alluminum") {
        material = std::make_unique<ConductiveMaterial>(ConductiveMaterial::alluminum(0.12, 0.2));
    }
    else if (material_type == "glass") {
        material = std::make_unique<DielectricMaterial>(spectra::GLASS_BK7_IOR());
    }
    else {
        std::cerr << "Unknown material type: " << material_type << std::endl;
        return 1;
    }

    if (!parser.check()) {
        parser.printErrors();
        return 1;
    }

    Scene scene(initialize_device());

    auto light_spectrum = std::make_shared<RGBIlluminantSpectrum>(RGB(3.0, 1.0, 2.0));
    scene.add_light(std::make_unique<PointLight>(
        Pt3(4., 6., 8.),
        light_spectrum,
        50.0f
    ));
    Transform transform =
        Transform::translation(position)
        * Transform::rotate_x(rotation.x)
        * Transform::rotate_y(rotation.y)
        * Transform::rotate_z(rotation.z)
        * Transform::scale(scale);
    scene.add_obj(filename, material.get(), transform);

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
        width, height, M_PI / 3.0,
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