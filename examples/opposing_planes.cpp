#include <iostream>

#include "camera.hpp"
#include "render.hpp"
#include "vec.hpp"

int main() {
    Scene scene(initialize_device());

    // scene.add_light(std::make_unique<PointLight>(
    //     Pt3(-2., -1., 1.),
    //     std::make_shared<RGBIlluminantSpectrum>(RGB(6.0, 0.2, 0.2)),
    //     40.0f
    // ));
    // scene.add_light(std::make_unique<PointLight>(
    //     Pt3(-2., 1., 1.),
    //     std::make_shared<RGBIlluminantSpectrum>(RGB(0.2, 0.2, 6.0)),
    //     60.0f
    // ));
    scene.add_light(std::make_unique<AreaLight>(
        std::make_unique<Quad>(
            Pt3(-12, -4, 4),
            Vec3(0, 12, 2),
            Vec3(12, 0, 0)
        ),
        std::make_shared<RGBIlluminantSpectrum>(RGB(2.5, 0.9, 1.8)),
        2.0f
    ));

    auto right_plane = ConductiveMaterial::copper(0.4, 0.2);
    scene.add_plane(
        Pt3(0., 0., -10.),
        Vec3(-0.5, 0.5, 1.).normalized(),
        &right_plane
    );

    auto left_plane = ConductiveMaterial::alluminum(0.3, 0.6);
    scene.add_plane(
        Pt3(0., 0., -10.),
        Vec3(0.5, -0.5, 1.).normalized(),
        &left_plane
    );

    DielectricMaterial sphere(spectra::GLASS_SF11_IOR());
    scene.add_sphere(
        Pt3(1., 1., -5.), 0.8,
        &sphere
    );
    scene.add_sphere(
        Pt3(0., 0., -6.), 0.8,
        &sphere
    );
    scene.add_sphere(
        Pt3(-1., -1., -7.), 0.8,
        &sphere
    );
    scene.commit();

    Camera camera(1920 * 2, 1080 * 2, M_PI_4);
    size_t n_samples = 256;
    size_t max_bounces = 64;

    auto result = render(
        camera,
        scene,
        n_samples, max_bounces
    );

    result.save("opposing_planes_no_denoise.png");
    result.save_albedo("opposing_planes_albedo.png");
    result.denoise();
    result.save("opposing_planes.png");

    return 0;
}