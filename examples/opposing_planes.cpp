#include <chrono>
#include <iostream>

#include "camera.hpp"
#include "render.hpp"
#include "vec.hpp"

int main() {
    Scene scene(initialize_device());

    // SimpleLight light(
    //     std::make_shared<RGBIlluminantSpectrum>(RGB(2.6, 0.8, 1.2))
    // );
    // scene.add_plane(
    //     Pt3(0., 0., 5.),
    //     Vec3(-1., 0., -1.).normalize(),
    //     nullptr, &light
    // );

    ConductiveMaterial right_plane(
        std::make_shared<ConstantSpectrum>(1.2),
        std::make_shared<RGBUnboundedSpectrum>(RGB(221. / 255., 15. / 255., 110. / 255.))
    );
    scene.add_plane(
        Pt3(0., 0., -10.),
        Vec3(-0.5, 0.5, 1.).normalize(),
        &right_plane
    );

    ConductiveMaterial left_plane(
        std::make_shared<ConstantSpectrum>(1.2),
        std::make_shared<RGBUnboundedSpectrum>(RGB(19. / 255., 70. / 255., 180. / 255.))
    );
    scene.add_plane(
        Pt3(0., 0., -10.),
        Vec3(0.5, -0.5, 1.).normalize(),
        &left_plane
    );

    DielectricMaterial sphere(1.2);
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
    result.denoise();
    result.save("opposing_planes.png");

    return 0;
}