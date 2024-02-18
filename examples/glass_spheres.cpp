#include <iostream>
#include <chrono>

#include "camera.hpp"
#include "image.hpp"
#include "render.hpp"
#include "scene.hpp"
#include "vec.hpp"


int main() {
    Scene scene(initialize_device());

    auto light_spectrum = 
        spectra::ILLUM_D65();
    auto light_shape = std::make_unique<Quad>(
        Pt3(-3.f, -3.f, -12.f),
        Vec3(6.f, 0.f, 0.f),
        Vec3(0.f, 6.f, 0.f)
    );
    scene.add_light(
        std::make_unique<AreaLight>(
            std::move(light_shape),
            light_spectrum,
            8.0f,
            false
        )
    );

    DielectricMaterial dielectric(spectra::GLASS_SF11_IOR());
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            for (int k = 0; k < 4; k++) {   
                scene.add_sphere(Pt3(-1.5 + i, -1.5 + j, -6 + k), 0.45, &dielectric);
            }
        }
    }
    scene.add_sphere(
        Pt3(-0.8f, -1.25f, -4.4f), 0.75f,
        &dielectric
    );
    
    scene.commit();

    Camera camera(
        800, 800, M_PI / 3.0f
    );
    size_t n_samples = 512;
    size_t max_bounces = 64;

    std::cout << "Rendering " << camera.image_height * camera.image_width << " pixels with " <<
        n_samples << " samples and " << max_bounces << " bounces" << std::endl;

    auto result = render(
        camera,
        scene,
        n_samples, max_bounces
    );
    result.save("glass_spheres_no_denoise.png");

    Image result_im(result.height, result.width);
    result_im.color_buffer = result.color_buffer;
    result_im.denoise();
    result_im.save("glass_spheres_denoise.png");

    Image albedo_im(result.height, result.width);
    albedo_im.color_buffer = result.albedo_buffer;
    albedo_im.save("glass_spheres_albedo.png");

    Image normal_im(result.height, result.width);
    normal_im.color_buffer = result.normal_buffer;
    normal_im.save("glass_spheres_normal.png");
    
    result.denoise();
    result.save("glass_spheres_advanced_denoise.png");

    return 0;
}