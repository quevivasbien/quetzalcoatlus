#include <fstream>
#include <vector>

#include "scene.hpp"

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

void Scene::commit() {
    rtcCommitScene(m_scene);
    m_ready = true;
}

Vec2 get_sphere_uv(const Vec3& n) {
    float phi = atan2f(n.z, n.x) + M_PI;
    float u = phi / (2.0f * M_PI);
    if (u >= 1.0f) {
        u -= __FLT_EPSILON__;
    }
    float theta = acosf(n.y);
    float v = theta / M_PI;
    if (v >= 1.0f) {
        v -= __FLT_EPSILON__;
    }
    return Vec2(u, v);
}

std::optional<SceneIntersection> Scene::ray_intersect(
    const Ray& ray,
    const WavelengthSample& wavelengths,
    Sampler& sampler
) const {
    alignas(16) RTCRayHit rayhit;
    rayhit.ray.org_x = ray.o.x;
    rayhit.ray.org_y = ray.o.y;
    rayhit.ray.org_z = ray.o.z;
    rayhit.ray.dir_x = ray.d.x;
    rayhit.ray.dir_y = ray.d.y;
    rayhit.ray.dir_z = ray.d.z;
    rayhit.ray.tnear = 0.0001f;
    rayhit.ray.tfar = std::numeric_limits<float>::infinity();
    rayhit.ray.mask = -1;
    rayhit.ray.flags = 0;
    rayhit.hit.geomID = RTC_INVALID_GEOMETRY_ID;
    rayhit.hit.instID[0] = RTC_INVALID_GEOMETRY_ID;

    rtcIntersect1(m_scene, &rayhit);

    if (rayhit.hit.geomID == RTC_INVALID_GEOMETRY_ID) {
        return std::nullopt;
    }

    auto geom_data = get_geom_data(rayhit.hit.geomID);
    if (!geom_data) {
        std::cout << "Geometry data not found for intersected object" << std::endl;
        return std::nullopt;
    }
    Vec3 normal = Vec3(rayhit.hit.Ng_x, rayhit.hit.Ng_y, rayhit.hit.Ng_z).normalize();
    Vec2 uv(rayhit.hit.u, rayhit.hit.v);
    if ((*geom_data)->shape == Shape::SPHERE) {
        // embree doesn't have uv coordinates for spheres
        // need to calculate this manually
        uv = get_sphere_uv(normal);
    }

    ShapeIntersection isect(
        uv,
        normal,
        ray.at(rayhit.ray.tfar),
        ray.d.dot(normal) < 0.0f,
        wavelengths
    );
    const Material* material = (*geom_data)->material;

    return SceneIntersection(
        material->scatter(ray, isect, sampler),
        normal
    );
}

template <typename RHN, size_t N>
void build_rayhits(RHN& rayhits, const std::array<Ray, N>& rays, const std::array<int, N>& valid) {
    for (size_t i = 0; i < N; i++) {
        if (valid[i] == 0) {
            continue;
        }
        rayhits.ray.org_x[i] = rays[i].o.x;
        rayhits.ray.org_y[i] = rays[i].o.y;
        rayhits.ray.org_z[i] = rays[i].o.z;
        rayhits.ray.dir_x[i] = rays[i].d.x;
        rayhits.ray.dir_y[i] = rays[i].d.y;
        rayhits.ray.dir_z[i] = rays[i].d.z;
        rayhits.ray.tnear[i] = 0.0001f;
        rayhits.ray.tfar[i] = std::numeric_limits<float>::infinity();
        rayhits.ray.mask[i] = -1;
        rayhits.ray.flags[i] = 0;
        rayhits.hit.geomID[i] = RTC_INVALID_GEOMETRY_ID;
        rayhits.hit.instID[0][i] = RTC_INVALID_GEOMETRY_ID;
    }
}

template <typename RHN, size_t N>
std::array<std::optional<SceneIntersection>, N> rayhit_result(
    const Scene* scene,
    const RHN& rayhits,
    const std::array<Ray, N>& rays,
    const std::array<int, N>& valid,
    const std::array<WavelengthSample, N>& wavelengths,
    Sampler& sampler
) {
    std::array<std::optional<SceneIntersection>, N> result;
    for (size_t i = 0; i < N; i++) {
        if (valid[i] == 0) {
            continue;
        }
        auto geom_id = rayhits.hit.geomID[i];
        if (geom_id == RTC_INVALID_GEOMETRY_ID) {
            continue;
        }
        auto geom_data = scene->get_geom_data(geom_id);
        if (!geom_data) {
            std::cout << "Geometry data not found for intersected object" << std::endl;
            continue;
        }
        Vec3 normal = Vec3(rayhits.hit.Ng_x[i], rayhits.hit.Ng_y[i], rayhits.hit.Ng_z[i]).normalize();
        Vec2 uv(rayhits.hit.u[i], rayhits.hit.v[i]);
        if ((*geom_data)->shape == Shape::SPHERE) {
            uv = get_sphere_uv(normal);
        }
        ShapeIntersection isect(
            uv,
            normal,
            rays[i].at(rayhits.ray.tfar[i]),
            rays[i].d.dot(normal) < 0.0f,
            wavelengths[i]
        );
        const Material* material = (*geom_data)->material;
        result[i] = SceneIntersection(
            material->scatter(rays[i], isect, sampler),
            normal
        );
    }

    return result;
}

std::array<std::optional<SceneIntersection>, 4> Scene::ray_intersect(
    const std::array<Ray, 4>& rays,
    const std::array<WavelengthSample, 4>& wavelengths,
    Sampler& sampler,
    const std::array<int, 4>& valid
) const {
    alignas(16) RTCRayHit4 rayhits;
    build_rayhits(rayhits, rays, valid);
    rtcIntersect4(valid.data(), m_scene, &rayhits);
    return rayhit_result(this, rayhits, rays, valid, wavelengths, sampler);
}

std::array<std::optional<SceneIntersection>, 8> Scene::ray_intersect(
    const std::array<Ray, 8>& rays,
    const std::array<WavelengthSample, 8>& wavelengths,
    Sampler& sampler,
    const std::array<int, 8>& valid
) const {
    alignas(32) RTCRayHit8 rayhits;
    build_rayhits(rayhits, rays, valid);
    rtcIntersect8(valid.data(), m_scene, &rayhits);
    return rayhit_result(this, rayhits, rays, valid, wavelengths, sampler);
}

unsigned int Scene::add_triangle(const Pt3& a, const Pt3& b, const Pt3& c, const Material* material) {
    RTCGeometry geom = rtcNewGeometry(
        m_device,
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
        vertices[0] = a.x; vertices[1] = a.y; vertices[2] = a.z;
        vertices[3] = b.x; vertices[4] = b.y; vertices[5] = b.z;
        vertices[6] = c.x; vertices[7] = c.y; vertices[8] = c.z;

        indices[0] = 0;
        indices[1] = 1;
        indices[2] = 2;
    }
    else {
        printf("Something went wrong when making triangle\n");
    }
    m_geom_data.push_back({ Shape::TRIANGLE, material });
    rtcSetGeometryUserData(geom, &m_geom_data.back());

    rtcCommitGeometry(geom);
    unsigned int geom_id = rtcAttachGeometry(m_scene, geom);
    rtcReleaseGeometry(geom);

    return geom_id;
}

unsigned int Scene::add_quad(
    const Pt3& a,
    const Pt3& b,
    const Pt3& c,
    const Pt3& d,
    const Material* material
) {
    RTCGeometry geom = rtcNewGeometry(
        m_device,
        RTC_GEOMETRY_TYPE_QUAD
    );
    float* vertices = static_cast<float*>(rtcSetNewGeometryBuffer(
        geom,
        RTC_BUFFER_TYPE_VERTEX,
        0,
        RTC_FORMAT_FLOAT3,
        3 * sizeof(float),
        4
    ));
    unsigned int* indices = static_cast<unsigned int*>(rtcSetNewGeometryBuffer(
        geom,
        RTC_BUFFER_TYPE_INDEX,
        0,
        RTC_FORMAT_UINT4,
        4 * sizeof(unsigned int),
        1
    ));
    
    if (vertices && indices) {
        vertices[0] = a.x; vertices[1] = a.y; vertices[2] = a.z;
        vertices[3] = b.x; vertices[4] = b.y; vertices[5] = b.z;
        vertices[6] = c.x; vertices[7] = c.y; vertices[8] = c.z;
        vertices[9] = d.x; vertices[10] = d.y; vertices[11] = d.z;

        indices[0] = 0;
        indices[1] = 1;
        indices[2] = 2;
        indices[3] = 3;
    }
    else {
        printf("Something went wrong when making quad\n");
    }
    m_geom_data.push_back({ Shape::QUAD, material });
    rtcSetGeometryUserData(geom, &m_geom_data.back());

    rtcCommitGeometry(geom);
    unsigned int geom_id = rtcAttachGeometry(m_scene, geom);
    rtcReleaseGeometry(geom);

    auto x = get_geom_data(geom_id);

    return geom_id;
}

unsigned int Scene::add_plane(const Pt3& p, const Vec3& n, const Material* material, float half_size) {
    // plane will be modeled as a large quad centered around the given point
    
    OrthonormalBasis basis(n);
    Pt3 a = p - basis.u[0] * half_size - basis.u[1] * half_size;
    Pt3 b = p + basis.u[0] * half_size - basis.u[1] * half_size;
    Pt3 c = p + basis.u[0] * half_size + basis.u[1] * half_size;
    Pt3 d = p - basis.u[0] * half_size + basis.u[1] * half_size;
    
    return add_quad(a, b, c, d, material); 
}

unsigned int Scene::add_sphere(const Pt3& center, float radius, const Material* material) {
    RTCGeometry geom = rtcNewGeometry(
        m_device,
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
    m_geom_data.push_back({ Shape::SPHERE, material });
    rtcSetGeometryUserData(geom, &m_geom_data.back());

    rtcCommitGeometry(geom);
    unsigned int geom_id = rtcAttachGeometry(m_scene, geom);
    rtcReleaseGeometry(geom);

    return geom_id;
}

unsigned int Scene::add_obj(const std::string& filename, const Material* material, const Transform& transform) {
    // for now, only supports triangle meshes
    std::vector<Pt3> vertices;
    std::vector<std::array<unsigned int, 3>> faces;
    // open file and read in vertices and faces
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cout << "Unable to open file" << std::endl;
        return RTC_INVALID_GEOMETRY_ID;
    }
    std::string line;
    while (std::getline(file, line)) {
        if (line.size() == 0 || line[0] == '#') {
            continue;
        }
        auto first_space = line.find(' ');
        if (first_space == std::string::npos) {
            std::cout << "Unrecognized format in line: " << line << std::endl;
            continue;
        }
        std::string first_word = line.substr(0, first_space);
        if (first_word == "v") {
            Pt3 v;
            std::sscanf(line.c_str(), "v %f %f %f", &v.x, &v.y, &v.z);
            vertices.push_back(transform * v);
        }
        else if (first_word == "f") {
            std::array<unsigned int, 3> f;
            if (line.find('/') == std::string::npos) {
                std::sscanf(line.c_str(), "f %u %u %u", &f[0], &f[1], &f[2]);
            }
            else {
                // try format f a/[at]/[an] b/[bt]/[bn] c/[ct]/[cn]
                // only save a b c
                unsigned int discard;
                int count = std::sscanf(line.c_str(), "f %u/%u/%u %u/%u/%u %u/%u/%u", &f[0], &discard, &discard, &f[1], &discard, &discard, &f[2], &discard, &discard);
                if (count < 9) {
                    std::sscanf(line.c_str(), "f %u//%u %u//%u %u//%u", &f[0], &discard, &f[1], &discard, &f[2], &discard);
                }
            }
            faces.push_back(f);
        }
        else {
            // std::cout << "Unrecognized format in line: " << line << std::endl;
            continue;
        }
    }
    file.close();

    RTCGeometry geom = rtcNewGeometry(
        m_device,
        RTC_GEOMETRY_TYPE_TRIANGLE
    );
    float* vertex_buf = static_cast<float*>(rtcSetNewGeometryBuffer(
        geom,
        RTC_BUFFER_TYPE_VERTEX,
        0,
        RTC_FORMAT_FLOAT3,
        3 * sizeof(float),
        vertices.size()
    ));
    unsigned int* indices = static_cast<unsigned int*>(rtcSetNewGeometryBuffer(
        geom,
        RTC_BUFFER_TYPE_INDEX,
        0,
        RTC_FORMAT_UINT3,
        3 * sizeof(unsigned int),
        faces.size()
    ));

    if (vertex_buf && indices) {
        for (size_t i = 0; i < vertices.size(); i++) {
            vertex_buf[i * 3 + 0] = vertices[i].x;
            vertex_buf[i * 3 + 1] = vertices[i].y;
            vertex_buf[i * 3 + 2] = vertices[i].z;
        }
        for (size_t i = 0; i < faces.size(); i++) {
            indices[i * 3 + 0] = faces[i][0] - 1;
            indices[i * 3 + 1] = faces[i][1] - 1;
            indices[i * 3 + 2] = faces[i][2] - 1;
        }
    }
    else {
        printf("Something went wrong when making obj\n");
    }

    m_geom_data.push_back({ Shape::OBJ, material });
    rtcSetGeometryUserData(geom, &m_geom_data.back());

    rtcCommitGeometry(geom);
    unsigned int geom_id = rtcAttachGeometry(m_scene, geom);
    rtcReleaseGeometry(geom);

    return geom_id;
}