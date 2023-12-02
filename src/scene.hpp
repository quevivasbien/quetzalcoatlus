#include <embree4/rtcore.h>
#include <stdio.h>
#include <limits>

#include "ray.hpp"
#include "shape.hpp"
#include "vec.hpp"

void error_function(void* userPtr, enum RTCError error, const char* str)
{
    printf("error %d: %s\n", error, str);
}

RTCDevice initialize_device() {
    RTCDevice device = rtcNewDevice(NULL);
    if (!device) {
        printf(
            "error %d: cannot create device\n", rtcGetDeviceError(NULL)
        );
    }

    rtcSetDeviceErrorFunction(device, error_function, NULL);
    return device;
}

struct GeomIntersection {
    float t;
    Vec2 uv;
    Vec3 normal;
    Pt3 point;
    bool outer_face;
    unsigned int geom_id;

    GeomIntersection(float t, Vec2 uv, Vec3 normal, Pt3 point, bool outer_face, unsigned int geom_id) : t(t), uv(uv), normal(normal), point(point), outer_face(outer_face), geom_id(geom_id) {}
};

class Scene {
public:
    explicit Scene(RTCDevice&& device) : device(device), scene(rtcNewScene(device)) {}

    ~Scene() {
        rtcReleaseScene(scene);
        rtcReleaseDevice(device);
    }

    void commit() {
        rtcCommitScene(scene);
        ready = true;
    }

    std::optional<GeomIntersection> ray_intersect(const Ray& ray) {
        RTCRayHit rayhit;
        rayhit.ray.org_x = ray.o.x;
        rayhit.ray.org_y = ray.o.y;
        rayhit.ray.org_z = ray.o.z;
        rayhit.ray.dir_x = ray.d.x;
        rayhit.ray.dir_y = ray.d.y;
        rayhit.ray.dir_z = ray.d.z;
        rayhit.ray.tnear = 0;
        rayhit.ray.tfar = std::numeric_limits<float>::infinity();
        rayhit.ray.mask = -1;
        rayhit.ray.flags = 0;
        rayhit.hit.geomID = RTC_INVALID_GEOMETRY_ID;
        rayhit.hit.instID[0] = RTC_INVALID_GEOMETRY_ID;

        rtcIntersect1(scene, &rayhit);

        if (rayhit.hit.geomID == RTC_INVALID_GEOMETRY_ID) {
            return std::nullopt;
        }

        float t = rayhit.ray.tfar;
        Vec2 uv(rayhit.hit.u, rayhit.hit.v);
        Vec3 normal(rayhit.hit.Ng_x, rayhit.hit.Ng_y, rayhit.hit.Ng_z);
        Pt3 point = ray.at(t);
        bool outer_face = ray.d.dot(normal) < 0.f;

        return GeomIntersection(
            t,
            uv,
            normal,
            point,
            outer_face,
            rayhit.hit.geomID
        );
    }

    RTCScene scene;
    RTCDevice device;
    bool ready = false;
};

void add_triangle(Scene& scene, Pt3 a, Pt3 b, Pt3 c) {
    RTCGeometry geom = rtcNewGeometry(
        scene.device,
        RTC_GEOMETRY_TYPE_TRIANGLE
    );
    float* vertices = static_cast<float*>(rtcSetNewGeometryBuffer(
        geom,
        RTC_BUFFER_TYPE_VERTEX,
        0,
        RTC_FORMAT_FLOAT3,
        3 * sizeof(float),
        3
    ));
    unsigned int* indices = static_cast<unsigned int*>(rtcSetNewGeometryBuffer(
        geom,
        RTC_BUFFER_TYPE_INDEX,
        0,
        RTC_FORMAT_UINT3,
        3 * sizeof(unsigned int),
        1
    ));
    
    if (vertices && indices) {
        vertices[0] = a.x;
        vertices[1] = a.y;
        vertices[2] = a.z;
        vertices[3] = b.x;
        vertices[4] = b.y;
        vertices[5] = b.z;
        vertices[6] = c.x;
        vertices[7] = c.y;
        vertices[8] = c.z;

        indices[0] = 0;
        indices[1] = 1;
        indices[2] = 2;
    }
    else {
        printf("Something went wrong when making triangle\n");
    }

    rtcCommitGeometry(geom);
    rtcAttachGeometry(scene.scene, geom);
    rtcReleaseGeometry(geom);
}

void add_sphere(Scene& scene, Pt3 center, float radius) {
    RTCGeometry geom = rtcNewGeometry(
        scene.device,
        RTC_GEOMETRY_TYPE_SPHERE_POINT
    );
    float* vertices = static_cast<float*>(rtcSetNewGeometryBuffer(
        geom,
        RTC_BUFFER_TYPE_VERTEX,
        0,
        RTC_FORMAT_FLOAT4,
        4 * sizeof(float),
        1
    ));

    if (vertices) {
        vertices[0] = center.x;
        vertices[1] = center.y;
        vertices[2] = center.z;
        vertices[3] = radius;
    }
    else {
        printf("Something went wrong when making sphere\n");
    }

    rtcCommitGeometry(geom);
    rtcAttachGeometry(scene.scene, geom);
    rtcReleaseGeometry(geom);
}