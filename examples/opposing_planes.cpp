#include <chrono>
#include <iostream>

#include "camera.hpp"
#include "render.hpp"
#include "vec.hpp"

int main() {
    Scene scene(initialize_device());

    scene.add_light(std::make_unique<PointLight>(
        Pt3(-2., 0., 1.),
        std::make_shared<RGBIlluminantSpectrum>(RGB(6.0, 2.0, 4.0)),
        100.0f
    ));

    auto right_plane = MixedMaterial<2>(
        {std::make_unique<ConductiveMaterial>(ConductiveMaterial::copper()), std::make_unique<DiffuseMaterial>(SolidColor(221. / 255., 15. / 255., 110. / 255.))},
        {0.9, 0.1}
    );
    scene.add_plane(
        Pt3(0., 0., -10.),
        Vec3(-0.5, 0.5, 1.).normalize(),
        &right_plane
    );

    auto left_plane = MixedMaterial<2>(
        {std::make_unique<ConductiveMaterial>(ConductiveMaterial::alluminum()), std::make_unique<DiffuseMaterial>(SolidColor(19. / 255., 70. / 255., 180. / 255.))},
        {0.9, 0.1}
    );
    scene.add_plane(
        Pt3(0., 0., -10.),
        Vec3(0.5, -0.5, 1.).normalize(),
        &left_plane
    );

    DielectricMaterial sphere(spectra::GLASS_SF11_IOR());
    // DiffuseMaterial sphere(SolidColor(0.8, 0.8, 0.8));
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

    Camera camera(1920, 1080, M_PI_4);
    size_t n_samples = 36;
    size_t max_bounces = 64;

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

    result.save("opposing_planes_no_denoise.png");
    result.save_albedo("opposing_planes_albedo.png");
    result.denoise();
    result.save("opposing_planes.png");

    return 0;
}