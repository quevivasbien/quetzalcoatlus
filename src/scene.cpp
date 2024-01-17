#include <fstream>
#include <vector>

#include "obj/obj.hpp"
#include "scene.hpp"

void error_function(void* userPtr, enum RTCError error, const char* str)
{
    std::cerr << "error: " << error << ": " << str << std::endl;
}

RTCDevice initialize_device() {
    RTCDevice device = rtcNewDevice(NULL);
    if (!device) {
        std::cerr << "error code " << rtcGetDeviceError(device) << ": cannot create device" << std::endl;
    }

    rtcSetDeviceErrorFunction(device, error_function, NULL);
    return device;
}

void Scene::commit() {
    rtcCommitScene(m_scene);
    m_ready = true;
}

Vec2 get_sphere_uv(const Vec3& n) {
    float phi = std::atan2(n.z, n.x) + M_PI;
    float u = phi / (2.0f * M_PI);
    if (u >= 1.0f) {
        u -= __FLT_EPSILON__;
    }
    float theta = std::acos(n.y);
    float v = theta / M_PI;
    if (v >= 1.0f) {
        v -= __FLT_EPSILON__;
    }
    return Vec2(u, v);
}

RTCRayHit create_rayhit(const Ray& ray, RTCScene scene) {
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

    rtcIntersect1(scene, &rayhit);

    return rayhit;
}

std::optional<SurfaceInteraction> Scene::ray_intersect(
    const Ray& ray,
    const WavelengthSample& wavelengths,
    Sampler& sampler
) const {
    auto rayhit = create_rayhit(ray, m_scene);

    if (rayhit.hit.geomID == RTC_INVALID_GEOMETRY_ID) {
        return std::nullopt;
    }

    auto geom_data = get_geom_data(rayhit.hit.geomID);
    if (!geom_data) {
        std::cout << "Geometry data not found for intersected object" << std::endl;
        return std::nullopt;
    }

    auto shape = geom_data->shape;
    auto material = geom_data->material;
    auto light = geom_data->light;
    auto normal_data = geom_data->normals.get();
    Vec2 uv(rayhit.hit.u, rayhit.hit.v);

    Vec3 normal;
    if (normal_data && shape == ShapeType::OBJ) {
        auto face_indices = normal_data->faces[rayhit.hit.primID];
        Vec3 v0 = normal_data->normals[face_indices[0]];
        Vec3 v1 = normal_data->normals[face_indices[1]];
        Vec3 v2 = normal_data->normals[face_indices[2]];
        Vec3 v3 = normal_data->normals[face_indices[3]];
        // use uv coordinates to interpolate over vertex normals
        normal = (
            (1.0f - uv.x) * (1.0f - uv.y) * v0
            + uv.x * (1.0f - uv.y) * v1
            + uv.x * uv.y * v2
            + (1.0f - uv.x) * uv.y * v3
        ).normalized();
    }
    else {
        normal = Vec3(rayhit.hit.Ng_x, rayhit.hit.Ng_y, rayhit.hit.Ng_z).normalized();
    }

    if (shape == ShapeType::SPHERE) {
        // embree doesn't have uv coordinates for spheres
        // need to calculate this manually
        uv = get_sphere_uv(normal);
    }

    return SurfaceInteraction(
        ray.at(rayhit.ray.tfar),
        (-ray.d).normalized(),
        normal,
        uv,
        material,
        light
    );
}

std::pair<const Light*, float> Scene::sample_lights(
    const Pt3& point, const Vec3& normal,
    Sampler& sampler
) const {
    if (m_lights.empty()) {
        return {nullptr, 0.0f};
    }
    // randomly select a light to sample from
    float u = sampler.sample_1d();
    size_t i = static_cast<size_t>(u * m_lights.size());
    return {m_lights[i].get(), 1.0f / m_lights.size()};
}

float Scene::light_sample_pmf(const Pt3& point, const Vec3& normal, const Light* light) const {
    return 1.0f / m_lights.size();
}

bool Scene::occluded(Pt3 start, Pt3 end) const {
    Ray ray(start, end - start);
    auto rayhit = create_rayhit(ray, m_scene);
    if (rayhit.hit.geomID == RTC_INVALID_GEOMETRY_ID) {
        return false;
    }
    return rayhit.ray.tfar <= 1.0f;
}

GeometryData* Scene::add_triangle(const Pt3& a, const Pt3& b, const Pt3& c, const Material* material) {
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
        std::cerr << "Something went wrong when making triangle" << std::endl;
    }
    m_geom_data.push_back({ ShapeType::TRIANGLE, material });
    GeometryData* geom_data = &m_geom_data.back();
    rtcSetGeometryUserData(geom, geom_data);

    rtcCommitGeometry(geom);
    rtcAttachGeometry(m_scene, geom);
    rtcReleaseGeometry(geom);

    return geom_data;
}

GeometryData* Scene::add_quad(
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
        std::cerr << "Something went wrong when making quad" << std::endl;
    }
    m_geom_data.push_back({ ShapeType::QUAD, material });
    GeometryData* geom_data = &m_geom_data.back();
    rtcSetGeometryUserData(geom, geom_data);

    rtcCommitGeometry(geom);
    rtcAttachGeometry(m_scene, geom);
    rtcReleaseGeometry(geom);

    return geom_data;
}

GeometryData* Scene::add_plane(const Pt3& p, const Vec3& n, const Material* material, float half_size) {
    // plane will be modeled as a large quad centered around the given point
    
    OrthonormalBasis basis(n);
    Pt3 a = p - basis.u[0] * half_size - basis.u[1] * half_size;
    Pt3 b = p + basis.u[0] * half_size - basis.u[1] * half_size;
    Pt3 c = p + basis.u[0] * half_size + basis.u[1] * half_size;
    Pt3 d = p - basis.u[0] * half_size + basis.u[1] * half_size;
    
    return add_quad(a, b, c, d, material); 
}

GeometryData* Scene::add_sphere(const Pt3& center, float radius, const Material* material) {
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
        std::cerr << "Something went wrong when making sphere" << std::endl;
    }
    m_geom_data.push_back({ ShapeType::SPHERE, material });
    GeometryData* geom_data = &m_geom_data.back();
    rtcSetGeometryUserData(geom, geom_data);

    rtcCommitGeometry(geom);
    rtcAttachGeometry(m_scene, geom);
    rtcReleaseGeometry(geom);

    return geom_data;
}

GeometryData* Scene::add_obj(const std::string& filename, const Material* material, const Transform& transform) {
    auto obj_data = obj::load_obj(filename);
    if (!obj_data) {
        std::cerr << "Failed to load " << filename << std::endl;
        return nullptr;
    }
    auto obj = *obj_data;
    if (obj.vertices.empty() || obj.faces.empty()) {
        return nullptr;
    }
    // use a quad mesh since the mesh may have both triangle and quad faces
    RTCGeometry geom = rtcNewGeometry(
        m_device,
        RTC_GEOMETRY_TYPE_QUAD
    );
    float* vertex_buf = static_cast<float*>(rtcSetNewGeometryBuffer(
        geom,
        RTC_BUFFER_TYPE_VERTEX,
        0,
        RTC_FORMAT_FLOAT3,
        3 * sizeof(float),
        obj.vertices.size()
    )); 
    unsigned int* indices = static_cast<unsigned int*>(rtcSetNewGeometryBuffer(
        geom,
        RTC_BUFFER_TYPE_INDEX,
        0,
        RTC_FORMAT_UINT4,
        4 * sizeof(unsigned int),
        obj.faces.size()
    ));

    if (!vertex_buf || !indices) {
        std::cerr << "Failed to create buffers for " << filename << std::endl;
        return nullptr;
    }

    for (size_t i = 0; i < obj.vertices.size(); i++) {
        Pt3 p = transform * Pt3(
            obj.vertices[i].x,
            obj.vertices[i].y,
            obj.vertices[i].z
        );
        vertex_buf[i * 3 + 0] = p.x;
        vertex_buf[i * 3 + 1] = p.y;
        vertex_buf[i * 3 + 2] = p.z;
    }

    for (size_t i = 0; i < obj.faces.size(); i++) {
        const auto& vertices = obj.faces[i].vertices;
        indices[i * 4 + 0] = vertices[0] - 1;
        indices[i * 4 + 1] = vertices[1] - 1;
        indices[i * 4 + 2] = vertices[2] - 1;
        // if the face is a tri, this last vertex will be a duplicate of the previous
        indices[i * 4 + 3] = vertices[3] - 1;
    }

    std::unique_ptr<NormalData> normal_data;
    if (!obj.vertex_normals.empty()) {
        std::vector<Vec3> normals(obj.vertex_normals.size());
        std::transform(obj.vertex_normals.begin(), obj.vertex_normals.end(), normals.begin(), [](const auto& v) {
            return Vec3(v.x, v.y, v.z);
        });
        std::vector<std::array<int, 4>> faces(obj.faces.size());
        std::transform(obj.faces.begin(), obj.faces.end(), faces.begin(), [](const auto& f) {
            auto ns = f.normals;
            return std::array<int, 4> { ns[0] - 1, ns[1] - 1, ns[2] - 1, ns[3] - 1 };
        });
        normal_data = std::make_unique<NormalData>(std::move(normals), std::move(faces));
    }

    m_geom_data.push_back({
        .shape = ShapeType::OBJ,
        .material = material,
        .normals = std::move(normal_data)
    });
    GeometryData* geom_data = &m_geom_data.back();
    rtcSetGeometryUserData(geom, geom_data);

    rtcCommitGeometry(geom);
    rtcAttachGeometry(m_scene, geom);
    rtcReleaseGeometry(geom);
    
    return geom_data;
}

GeometryData* Scene::add_grid(const Image& image, const Material* material, const Transform& transform) {
    auto geom = rtcNewGeometry(m_device, RTC_GEOMETRY_TYPE_GRID);
    float* vertex_buf = static_cast<float*>(rtcSetNewGeometryBuffer(
        geom,
        RTC_BUFFER_TYPE_VERTEX,
        0,
        RTC_FORMAT_FLOAT3,
        3 * sizeof(float),
        image.width * image.height
    ));
    RTCGrid* grid = static_cast<RTCGrid*>(rtcSetNewGeometryBuffer(
        geom,
        RTC_BUFFER_TYPE_GRID,
        0,
        RTC_FORMAT_GRID,
        sizeof(RTCGrid),
        1
    ));
    if (!vertex_buf || !grid) {
        std::cerr << "Failed to create buffers for grid" << std::endl;
        return nullptr;
    }
    for (size_t i = 0; i < image.width * image.height; i++) {
        Pt3 p = transform * Pt3(
            image.color_buffer[i * 3 + 0],
            image.color_buffer[i * 3 + 1],
            image.color_buffer[i * 3 + 2]
        );
        vertex_buf[i * 3 + 0] = p.x;
        vertex_buf[i * 3 + 1] = p.y;
        vertex_buf[i * 3 + 2] = p.z;
    }
    if (image.width > std::numeric_limits<unsigned short>::max() || image.height > std::numeric_limits<unsigned short>::max()) {
        std::cerr << "Grid too large" << std::endl;
        return nullptr;
    }
    grid[0] = RTCGrid {
        .startVertexID = 0,
        .stride = static_cast<unsigned int>(image.width),
        .width = static_cast<unsigned short>(image.width),
        .height = static_cast<unsigned short>(image.height)
    };

    m_geom_data.push_back({
        .shape = ShapeType::GRID,
        .material = material
    });
    GeometryData* geom_data = &m_geom_data.back();
    rtcSetGeometryUserData(geom, geom_data);

    rtcCommitGeometry(geom);
    rtcAttachGeometry(m_scene, geom);
    rtcReleaseGeometry(geom);

    return geom_data;
}

void Scene::add_light(std::unique_ptr<Light>&& light) { 
    if (light->type() == LightType::AREA) {
        // add the light's geometry to the scene
        auto area_light = static_cast<const AreaLight*>(light.get());
        const Shape* shape = area_light->shape();
        auto shape_type = shape->type();
        if (shape_type == ShapeType::SPHERE) {
            const Sphere* sphere = static_cast<const Sphere*>(shape);
            auto geom_data = add_sphere(sphere->m_center, sphere->m_radius, nullptr);
            if (geom_data) {
                geom_data->light = area_light;
            }
        }
        else if (shape_type == ShapeType::QUAD) {
            const Quad* quad = static_cast<const Quad*>(shape);
            auto [a, b, c, d] = quad->get_vertices();
            auto geom_data = add_quad(a, b, c, d, nullptr);
            if (geom_data) {
                geom_data->light = area_light;
            }
        }
        else {
            std::cerr << "Shape type not yet supported as an area light: " << shape->type() << std::endl;
            return;
        }
    }
    m_lights.push_back(std::move(light));
}

void Scene::set_bg_light(std::shared_ptr<const Spectrum> spectrum, float scale) {
    m_bg_light.spectrum = spectrum;
    m_bg_light.scale = scale;
}