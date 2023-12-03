#pragma once

#include <cmath>

#include "image.hpp"
#include "vec.hpp"

class Texture {
public:
    virtual Vec3 value(const Vec2& uv, const Pt3& point) const = 0;
};


class SolidColor : public Texture {
public:
    explicit SolidColor(Vec3 color) : color(color) {}
    SolidColor(float r, float g, float b) : color(r, g, b) {}

    Vec3 value(const Vec2& uv, const Pt3& point) const override {
        return color;
    }

    Vec3 color;
};


// texture used in testing to display uv coordinates
class DummyTexture : public Texture {
public:
    Vec3 value(const Vec2& uv, const Pt3& point) const override {
        float u = uv.x * 10.;
        float v = uv.y * 10.;
        if (int(floorf(u) + floorf(v)) % 2 == 0) {
            return Vec3(1.0f, 1.0f, 1.0f);
        }
        else {
            return Vec3(0.0f, 0.0f, 0.0f);
        }
    }
};


class ImageTexture : public Texture {
public:
    explicit ImageTexture(Image&& image) : image(std::move(image)) {};

    Vec3 value(const Vec2& uv, const Pt3& point) const override {
        size_t x = uv.x * image.width;
        size_t y = uv.y * image.height;
        return Vec3(image.color_buffer[3 * (y * image.width + x) + 0],
                    image.color_buffer[3 * (y * image.width + x) + 1],
                    image.color_buffer[3 * (y * image.width + x) + 2]);
    }

    Image image;
};