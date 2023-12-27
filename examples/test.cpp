#include "render.hpp"

int main() {
    Scene scene(initialize_device());
    scene.add_light(std::make_unique<PointLight>(
        Pt3(0., 0., 1.),
        spectra::ILLUM_D65(),
        10.0f
    ));

    // auto copper = std::make_unique<ConductiveMaterial>(ConductiveMaterial::copper(0.1, 0.01));
    // auto lambert = std::make_unique<DiffuseMaterial>(SolidColor(1.0, 1.0, 1.0));
    // MixedMaterial<2> material(
    //     {std::move(copper), std::move(lambert)},
    //     {0.5, 0.5}
    // );
    auto material = ConductiveMaterial::copper(0., 0.);
    scene.add_sphere(Pt3(0, 0, -2), 1, &material);

    scene.commit();

    Camera camera(
        100, 100, M_PI / 3.0
    );
    size_t n_samples = 24;
    size_t max_bounces = 64;

    auto result = render(camera, scene, n_samples, max_bounces);
    result.save("test.png");
}