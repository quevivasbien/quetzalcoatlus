#pragma once

#include <array>
#include <deque>
#include <iostream>
#include <limits>
#include <optional>

#include <embree4/rtcore.h>

#include "material.hpp"
#include "random.hpp"
#include "ray.hpp"
#include "vec.hpp"

RTCDevice initialize_device();

struct SceneIntersection : ScatterEvent {
    Vec3 normal;

    SceneIntersection(ScatterEvent&& se, Vec3 normal) : ScatterEvent(std::move(se)), normal(normal) {}
};

enum Shape {
    SPHERE,
    TRIANGLE,
    QUAD
};

struct GeometryData {
    Shape shape;
    const Material* material;
};

class Scene {
public:
    explicit Scene(RTCDevice&& device) : m_device(device), m_scene(rtcNewScene(device)) {}

    ~Scene() {
        rtcReleaseScene(m_scene);
        rtcReleaseDevice(m_device);
    }

    void commit();
    bool ready() const { return m_ready; }

    // intersect a single ray with the scene
    std::optional<SceneIntersection> ray_intersect(const Ray& ray, Sampler& sampler) const;
    // intersect a packet of 4 rays with the scene
    std::array<std::optional<SceneIntersection>, 4> ray_intersect(
        const std::array<Ray, 4>& rays,
        Sampler& sampler,
        const std::array<int, 4>& valid = { -1, -1, -1, -1 }
    ) const;
    // intersect a packet of 8 rays with the scene
    std::array<std::optional<SceneIntersection>, 8> ray_intersect(
        const std::array<Ray, 8>& rays,
        Sampler& sampler,
        const std::array<int, 8>& valid = { -1, -1, -1, -1, -1, -1, -1, -1 }
    ) const;

    // methods for adding shapes to scene; return geometry ID
    // in cases where multiple points are required, they should be given in clockwise order around the outward face

    unsigned int add_triangle(const Pt3& a, const Pt3& b, const Pt3& c, const Material* material);
    unsigned int add_sphere(const Pt3& center, float radius, const Material* material);
    unsigned int add_quad(const Pt3& a, const Pt3& b, const Pt3& c, const Pt3& d, const Material* material);
    // plane is just a large square quad centered around the given point
    unsigned int add_plane(const Pt3& p, const Vec3& n, const Material* material, float half_size = 1000.0f);

    std::optional<const GeometryData*> get_geom_data(unsigned int geom_id) const {
        const void* ptr = rtcGetGeometryUserDataFromScene(m_scene, geom_id);
        if (ptr == NULL) {
            return std::nullopt;
        }
        return static_cast<const GeometryData*>(ptr);
    }
    RTCScene get_scene() const {
        return m_scene;
    }
    RTCDevice get_device() const {
        return m_device;
    }


private:
    RTCScene m_scene;
    RTCDevice m_device;
    // need to store data in a collection that doesn't reallocate on resize
    // since we'll be providing our geom objects with pointers to it
    std::deque<GeometryData> m_geom_data;
    bool m_ready = false;
};
