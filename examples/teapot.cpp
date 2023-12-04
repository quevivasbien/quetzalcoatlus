#include "render.hpp"

int main() {
    Scene scene(initialize_device());
    EmissiveMaterial light(SolidColor(1.0, 1.0, 1.0));
    scene.add_plane(
        Pt3(0., 0., 5.),
        Vec3(-1., 0., -1.).normalize(),
        &light
    );
    LambertMaterial teapot(SolidColor(0.5, 0.5, 0.5));
    scene.add_obj("teapot.obj", &teapot);
    scene.commit();

    Camera camera(1920, 1080, 1.);
    size_t n_samples = 4;
    size_t max_bounces = 16;

    auto result = render(camera, scene, n_samples, max_bounces);
    result.save("teapot.png", 0.8);

    return 0;
}