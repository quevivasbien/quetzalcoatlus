#include "vec.hpp"

class Texture {
public:
    virtual Vec3 value(const Vec2& uv, const Pt3& point) const = 0;
};


class SolidColor : public Texture {
public:
    SolidColor(Vec3 color) : color(color) {}

    Vec3 value(const Vec2& uv, const Pt3& point) const override {
        return color;
    }

    Vec3 color;
};
