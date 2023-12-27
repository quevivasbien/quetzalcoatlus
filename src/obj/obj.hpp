#include <fstream>
#include <iostream>
#include <optional>
#include <regex>
#include <string>
#include <vector>

namespace obj {

struct Vertex {
    float x;
    float y;
    float z;
    float w = 1.0;

    static std::optional<Vertex> from_line(const std::string& line) {
        std::regex r("v\\s+(\\S+)\\s+(\\S+)\\s+(\\S+)(?:\\s+(\\S+))?");
        std::smatch match;
        if (!std::regex_search(line, match, r)) {
            return std::nullopt;
        }
        float x = std::stof(match[1].str());
        float y = std::stof(match[2].str());
        float z = std::stof(match[3].str());
        auto v = Vertex {
            x, y, z
        };
        if (match[4].matched) {
            v.w = std::stof(match[4].str());
        }
        return v;
    }
};

struct FaceElement {
    std::vector<int> vertices;
    std::vector<int> textures;
    std::vector<int> normals;

    static std::optional<FaceElement> from_line(const std::string& line) {
        std::regex r_only_vert("f\\s+(\\d+)\\s+(\\d+)\\s+(\\d+)");
        std::smatch match;
        if (std::regex_search(line, match, r_only_vert)) {
            int x = std::stoi(match[1].str());
            int y = std::stoi(match[2].str());
            int z = std::stoi(match[3].str());
            return FaceElement {
                .vertices = { x, y, z }
            };
        }
        std::regex r_vert_tex("f\\s+(\\d+)/(\\d+)\\s+(\\d+)/(\\d+)\\s+(\\d+)/(\\d+)");
        if (std::regex_search(line, match, r_vert_tex)) {
            return FaceElement {
                .vertices = {
                    std::stoi(match[1].str()),
                    std::stoi(match[3].str()),
                    std::stoi(match[5].str())
                },
                .textures = {
                    std::stoi(match[2].str()),
                    std::stoi(match[4].str()),
                    std::stoi(match[6].str())
                }
            };
        }
        std::regex r_vert_tex_norm("f\\s+(\\d+)/(\\d+)?/(\\d+)\\s+(\\d+)/(\\d+)?/(\\d+)\\s+(\\d+)/(\\d+)?/(\\d+)");
        if (std::regex_search(line, match, r_vert_tex_norm)) {
            auto fe = FaceElement {
                .vertices = {
                    std::stoi(match[1].str()),
                    std::stoi(match[4].str()),
                    std::stoi(match[7].str())
                },
                .normals = {
                    std::stoi(match[3].str()),
                    std::stoi(match[6].str()),
                    std::stoi(match[9].str())
                }
            };
            if (match[2].matched && match[5].matched && match[8].matched) {
                fe.textures = {
                    std::stoi(match[2].str()),
                    std::stoi(match[5].str()),
                    std::stoi(match[8].str())
                };
            }
            return fe;
        }
        return std::nullopt;
    }
};

struct Object {
    std::string name;
    std::vector<Vertex> vertices;
    std::vector<FaceElement> faces;
};

struct ObjData {
    std::vector<Object> objects;
};

std::optional<ObjData> load_obj(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cout << "Unable to open file " << filename << std::endl;
        return std::nullopt;
    }
    std::string line;
    // TODO: handle objects properly
    // TODO: allow textures and normals to be added afterward
    Object obj { "default" };
    while(std::getline(file, line)) {
        std::regex r_line_type("^\\S+");
        std::smatch match;
        auto res = std::regex_search(line, match, r_line_type);
        if (!res || match.str()[0] == '#') {
            continue;
        }
        auto line_type = match.str();
        if (line_type == "v") {
            auto vert = Vertex::from_line(line);
            if (vert) {
                obj.vertices.push_back(*vert);
            }
        }
        else if (line_type == "f") {
            auto face = FaceElement::from_line(line);
            if (face) {
                obj.faces.push_back(*face);
            }
        }
    }

    file.close();

    return ObjData { { obj } };
}

} // namespace obj
