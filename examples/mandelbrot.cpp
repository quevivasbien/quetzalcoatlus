#include "image.hpp"
#include "render.hpp"

const unsigned int MAX_ITERATIONS = 100;
const unsigned int N_ROWS = 800;
const unsigned int N_COLS = 800;

const float X_MIN = -2.0f;
const float X_MAX = 1.0f;
const float Y_MIN = -1.5f;
const float Y_MAX = 1.5f;

float mandel_iter(float x, float y) {
    float zx = 0.0f;
    float zy = 0.0f;
    float zx2 = 0.0f;
    float zy2 = 0.0f;
    unsigned int i = 0;
    while (zx2 + zy2 < 4.0f && i < MAX_ITERATIONS) {
        zx2 = zx * zx;
        zy2 = zy * zy;
        zx = zx2 - zy2 + x;
        zy = 2.0f * zx * zy + y;
        i++;
    }
    return std::pow(i / (float)MAX_ITERATIONS, 0.1);
}

Image mandelbrot() {
    Image out(N_ROWS, N_COLS);
    for (unsigned int i = 0; i < N_ROWS; i++) {
        for (unsigned int j = 0; j < N_COLS; j++) {
            float x = X_MIN + (X_MAX - X_MIN) * j / N_COLS;
            float y = Y_MIN + (Y_MAX - Y_MIN) * i / N_ROWS;
            auto base_idx = 3 * (i * N_COLS + j);
            out.color_buffer[base_idx + 0] = x;
            out.color_buffer[base_idx + 1] = y;
            out.color_buffer[base_idx + 2] = mandel_iter(x, y);
        }
    }

    return out;
}

int main() {
    Image image = mandelbrot();
    auto material = ConductiveMaterial::copper(0.2, 0.12);

    Scene scene(initialize_device());
    scene.add_light(std::make_unique<PointLight>(
        Pt3(0., 5., 5.),
        std::make_shared<RGBIlluminantSpectrum>(RGB(8., 2., 4.)),
        60.0f
    ));
    scene.add_grid(image, &material, Transform::translation(0.5, 0., -5.) * Transform::rotate_x(-0.5));
    scene.commit();

    Camera camera(400, 400, M_PI_4);

    auto result = render(camera, scene, 12, 12);
    result.save("mandelbrot.png");
    result.save_normal("mandelbrot_normals.png");

    return 0;
}