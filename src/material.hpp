#pragma once

#include "bxdf.hpp"
#include "color/color.hpp"
#include "random.hpp"
#include "ray.hpp"
#include "material.hpp"
#include "texture.hpp"
#include "vec.hpp"

struct SurfaceInteraction;

class Material {
public:
    virtual BSDF bsdf(const SurfaceInteraction& si, const WavelengthSample& wavelengths) const = 0;
};


class DiffuseMaterial : public Material {
public:
    explicit DiffuseMaterial(std::unique_ptr<Texture>&& texture) : m_texture(std::move(texture)) {}

    template <typename T>
    explicit DiffuseMaterial(T&& texture) : m_texture(std::make_unique<T>(std::forward<T>(texture))) {}

    BSDF bsdf(const SurfaceInteraction& si, const WavelengthSample& wavelengths) const override;

    std::unique_ptr<Texture> m_texture;
};
