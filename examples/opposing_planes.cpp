#include <iostream>

#include "camera.hpp"
#include "primitive.hpp"
#include "render.hpp"
#include "shape.hpp"
#include "vec.hpp"

int main() {
    ShapePrimitive light(
        Plane(Pt3(0., 5., 0.), Vec3(-1., 0., -1.).normalize()),
        EmissiveMaterial(SolidColor(1.0, 1.0, 1.0))
    );
    ShapePrimitive left_plane(
        Plane(Pt3(0., 0., -10.), Vec3(-0.5, 0.5, 1.).normalize()),
        SpecularMaterial(SolidColor(201. / 255., 10. / 255., 64. / 255.), 0.1)
    );
    ShapePrimitive right_plane(
        Plane(Pt3(0., 0., -10.), Vec3(0.5, -0.5, 1.).normalize()),
        SpecularMaterial(SolidColor(0., 138. / 255., 216. / 255.), 0.1)
    );
    ShapePrimitive sphere1(
        Sphere(Pt3(1., 1., -5.), 0.8),
        RefractiveMaterial(SolidColor(1., 1., 1.), 1.2)
    );
    ShapePrimitive sphere2(
        Sphere(Pt3(0., 0., -6.), 0.8),
        RefractiveMaterial(SolidColor(1., 1., 1.), 1.2)
    );
    ShapePrimitive sphere3(
        Sphere(Pt3(-1., -1., -7.), 0.8),
        RefractiveMaterial(SolidColor(1., 1., 1.), 1.2)
    );

    Aggregate world({&light, &left_plane, &right_plane, &sphere1, &sphere2, &sphere3});
    Camera camera(1920, 1080, 1.);
    size_t n_samples = 25;
    size_t max_bounces = 64;

    auto start_time = std::chrono::steady_clock::now();
    auto result = render(
        camera,
        world,
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