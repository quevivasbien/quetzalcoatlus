#pragma once

#include <iostream>
#include <limits>
#include <optional>
#include <vector>

#include <embree4/rtcore.h>

#include "material.hpp"
#include "random.hpp"
#include "ray.hpp"
#include "vec.hpp"

RTCDevice initialize_device();

struct WorldIntersection : ScatterEvent {
    Vec3 normal;

    WorldIntersection(ScatterEvent&& se, Vec3 normal) : ScatterEvent(std::move(se)), normal(normal) {}
};

class Scene {
public:
    explicit Scene(RTCDevice&& device) : device(device), scene(rtcNewScene(device)) {}

    ~Scene() {
        rtcReleaseScene(scene);
        rtcReleaseDevice(device);
    }

    void commit();

    std::optional<WorldIntersection> ray_intersect(const Ray& ray, Sampler& sampler) const;

    // methods for adding shapes to scene; return geometry ID
    // in cases where multiple points are required, they should be given in clockwise order around the outward face

    unsigned int add_triangle(const Pt3& a, const Pt3& b, const Pt3& c, const Material* material);
    unsigned int add_sphere(const Pt3& center, float radius, const Material* material);
    unsigned int add_quad(const Pt3& a, const Pt3& b, const Pt3& c, const Pt3& d, const Material* material);

    bool ready = false;

private:
    RTCScene scene;
    RTCDevice device;
    std::vector<const Material*> materials;
};
