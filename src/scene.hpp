#pragma once

#include <array>
#include <deque>
#include <iostream>
#include <limits>
#include <optional>
#include <string>

#include <embree4/rtcore.h>

#include "color/color.hpp"
#include "image.hpp"
#include "interaction.hpp"
#include "light.hpp"
#include "material.hpp"
#include "sampler.hpp"
#include "ray.hpp"
#include "vec.hpp"

RTCDevice initialize_device();

struct BackgroundLight {
    std::shared_ptr<const Spectrum> spectrum;
    float scale = 1.0f;
};

struct NormalData {
    // buffer of normal vectors for each vertex
    std::vector<Vec3> normals;
    // for each face, indices in the 'normals' buffer
    std::vector<std::array<int, 4>> faces;
};

struct GeometryData {
    ShapeType shape;
    const Material* material;
    const AreaLight* light;
    std::optional<MediumInterface> medium_interface;
    std::unique_ptr<NormalData> normals;
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
    std::optional<SurfaceInteraction> ray_intersect(const Ray& ray, float t_max = std::numeric_limits<float>::infinity()) const;

    // sample illumination from lights at a given point
    std::pair<const Light*, float> sample_lights(const Pt3& point, Sampler& sampler) const;
    // get proba of sampling a given light
    float light_sample_pmf(const Pt3& point, const Vec3& normal, const Light* light) const;
    // check if end is visible from start
    bool occluded(Pt3 start, Pt3 end) const;

    // methods for adding shapes to scene; return geometry ID
    // in cases where multiple points are required, they should be given in clockwise order around the outward face

    GeometryData* add_triangle(
        const Pt3& a, const Pt3& b, const Pt3& c,
        const Material* material,
        const std::optional<MediumInterface>& medium_interface = std::nullopt
    );
    GeometryData* add_sphere(
        const Pt3& center, float radius,
        const Material* material,
        const std::optional<MediumInterface>& = std::nullopt
    );
    GeometryData* add_quad(
        const Pt3& a, const Pt3& b, const Pt3& c, const Pt3& d,
        const Material* material,
        const std::optional<MediumInterface>& = std::nullopt
    );
    // plane is just a large square quad centered around the given point
    GeometryData* add_plane(
        const Pt3& p, const Vec3& n,
        const Material* material,
        const std::optional<MediumInterface>& = std::nullopt,
        float half_size = 1000.0f
    );
    // add objects from .obj (wavefront OBJ) file
    GeometryData* add_obj(
        const std::string& filename,
        const Material* material,
        const std::optional<MediumInterface>& = std::nullopt,
        const Transform& transform = Transform::identity()
    );

    GeometryData* add_grid(
        const Image& image,
        const Material* material,
        const std::optional<MediumInterface>& = std::nullopt,
        const Transform& transform = Transform::identity()
    );

    // add a light to the scene
    void add_light(std::unique_ptr<Light>&& light);

    // set properties of background (ambient) lighting
    void set_bg_light(std::shared_ptr<const Spectrum> spectrum, float scale = 1.0f);

    const GeometryData* get_geom_data(unsigned int geom_id) const {
        const void* ptr = rtcGetGeometryUserDataFromScene(m_scene, geom_id);
        return static_cast<const GeometryData*>(ptr);
    }

    RTCScene get_scene() const {
        return m_scene;
    }
    RTCDevice get_device() const {
        return m_device;
    }

    const BackgroundLight& get_bg_light () const { return m_bg_light; }

    bool has_media() const;

private:
    RTCScene m_scene;
    RTCDevice m_device;

    BackgroundLight m_bg_light;
    // need to store data in a collection that doesn't reallocate on resize
    // since we'll be providing our geom objects with pointers to it
    std::deque<GeometryData> m_geom_data;
    std::vector<std::unique_ptr<Light>> m_lights;
    bool m_ready = false;
};
