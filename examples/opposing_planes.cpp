#include <chrono>
#include <iostream>

#include "camera.hpp"
#include "render.hpp"
#include "vec.hpp"

int main() {
    Scene scene(initialize_device());

    EmissiveMaterial light(SolidColor(1.0, 1.0, 1.0));
    scene.add_plane(
        Pt3(0., 0., 5.),
        Vec3(-1., 0., -1.).normalize(),
        &light
    );

    SpecularMaterial left_plane(SolidColor(201. / 255., 10. / 255., 64. / 255.), 0.1);
    scene.add_plane(
        Pt3(0., 0., -10.),
        Vec3(-0.5, 0.5, 1.).normalize(),
        &left_plane
    );

    SpecularMaterial right_plane(SolidColor(0., 138. / 255., 216. / 255.), 0.1);
    scene.add_plane(
        Pt3(0., 0., -10.),
        Vec3(0.5, -0.5, 1.).normalize(),
        &right_plane
    );

    RefractiveMaterial sphere(SolidColor(1.0, 1.0, 1.0), 1.2);
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

    Camera camera(1920, 1080, 1.);
    size_t n_samples = 25;
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

    result.save("opposing_planes_no_denoise.png", 0.8);
    result.denoise();
    result.save("opposing_planes.png", 0.8);

    return 0;
}