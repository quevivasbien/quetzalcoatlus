#pragma once

#include <array>
#include <optional>
#include <string>
#include <vector>

namespace obj {

struct Vertex {
    float x;
    float y;
    float z;
    float w = 1.0;

    static std::optional<Vertex> from_line(const std::string& line);
};

struct VertexNormal {
    float x;
    float y;
    float z;

    static std::optional<VertexNormal> from_line(const std::string& line);
};

struct FaceElement {
    std::array<int, 4> vertices;
    std::array<int, 4> textures;
    std::array<int, 4> normals;

    size_t n_vertices;

    static std::optional<FaceElement> from_line(const std::string& line);
};

struct ObjData {
    std::vector<Vertex> vertices;
    std::vector<VertexNormal> vertex_normals;
    std::vector<FaceElement> faces;
};

std::optional<ObjData> load_obj(const std::string& filename);

} // namespace obj
