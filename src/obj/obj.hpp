#pragma once

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

struct FaceElement {
    std::vector<int> vertices;
    std::vector<int> textures;
    std::vector<int> normals;

    static std::optional<FaceElement> from_line(const std::string& line);
};

struct Object {
    std::string name;
    std::vector<Vertex> vertices;
    std::vector<FaceElement> faces;
};

struct ObjData {
    std::vector<Object> objects;
};

std::optional<ObjData> load_obj(const std::string& filename);

} // namespace obj
