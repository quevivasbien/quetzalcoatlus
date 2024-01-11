#include "obj.hpp"

#include <fstream>
#include <iostream>

#ifdef NO_BOOST
#include <regex>
#else
#include <boost/regex.hpp>
#endif

namespace obj {

#ifdef NO_BOOST
namespace regex = std;
#else
namespace regex = boost;
#endif

std::optional<Vertex> Vertex::from_line(const std::string& line) {
    regex::regex r("v\\s+(\\S+)\\s+(\\S+)\\s+(\\S+)(?:\\s+(\\S+))?");
    regex::smatch match;
    if (!regex::regex_search(line, match, r)) {
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

std::optional<FaceElement> FaceElement::from_line(const std::string& line) {
    regex::regex r_only_vert("f\\s+(\\d+)\\s+(\\d+)\\s+(\\d+)(?:\\s+(\\d+))?");
    regex::smatch match;
    if (regex::regex_search(line, match, r_only_vert)) {
        int a = std::stoi(match[1].str());
        int b = std::stoi(match[2].str());
        int c = std::stoi(match[3].str());
        if (match[4].matched) {
            int d = std::stoi(match[4].str());
            return FaceElement {
                .vertices = { a, b, c, d }
            };
        }
        return FaceElement {
            .vertices = { a, b, c }
        };
    }
    regex::regex r_vert_tex("f\\s+(\\d+)\\/(\\d+)\\s+(\\d+)\\/(\\d+)\\s+(\\d+)\\/(\\d+)(?:\\s+(\\d+)\\/(\\d+))?");
    if (regex::regex_search(line, match, r_vert_tex)) {
        int va = std::stoi(match[1].str());
        int ta = std::stoi(match[2].str());
        int vb = std::stoi(match[3].str());
        int tb = std::stoi(match[4].str());
        int vc = std::stoi(match[5].str());
        int tc = std::stoi(match[6].str());
        if (match[7].matched) {
            int vd = std::stoi(match[7].str());
            int td = std::stoi(match[8].str());
            return FaceElement {
                .vertices = { va, vb, vc, vd },
                .textures = { ta, tb, tc, td }
            };
        }
        return FaceElement {
            .vertices = { va, vb, vc },
            .textures = { ta, tb, tc }
        };
    }
    regex::regex r_vert_tex_norm("f\\s+(\\d+)\\/(\\d+)?\\/(\\d+)\\s+(\\d+)\\/(\\d+)?\\/(\\d+)\\s+(\\d+)\\/(\\d+)?\\/(\\d+)(?:\\s+(\\d+)\\/(\\d+)?\\/(\\d+))?");
    if (regex::regex_search(line, match, r_vert_tex_norm)) {
        int va = std::stoi(match[1].str());
        int na = std::stoi(match[3].str());
        int vb = std::stoi(match[4].str());
        int nb = std::stoi(match[6].str());
        int vc = std::stoi(match[7].str());
        int nc = std::stoi(match[9].str());
        FaceElement fe;
        if (match[10].matched) {
            int vd = std::stoi(match[10].str());
            int nd = std::stoi(match[12].str());
            fe.vertices = { va, vb, vc, vd };
            fe.normals = { na, nb, nc, nd };
        }
        else {
            fe.vertices = { va, vb, vc };
            fe.normals = { na, nb, nc };
        }
        if (match[2].matched && match[5].matched && match[8].matched) {
            fe.textures = {
                std::stoi(match[2].str()),
                std::stoi(match[5].str()),
                std::stoi(match[8].str())
            };
            if (match[11].matched) {
                fe.textures.push_back(std::stoi(match[11].str()));
            }
        }
        return fe;
    }
    return std::nullopt;
}

std::optional<ObjData> load_obj(const std::string& filename) {
    std::cerr << "Loading " << filename << "..." <<std::endl;
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cout << "Unable to open file " << filename << std::endl;
        return std::nullopt;
    }
    std::string line;
    // TODO: allow textures and normals to be added afterward
    ObjData obj_data { { Object { "default" } } };
    std::vector<Object>& objects = obj_data.objects;
    Object* current_obj = &objects.back();
    while(std::getline(file, line)) {
        regex::regex r_line_type("^\\S+");
        regex::smatch match;

        // skip comments
        auto res = regex::regex_search(line, match, r_line_type);
        if (!res || match.str()[0] == '#') {
            continue;
        
        }
        auto line_type = match.str();

        // todo: handle objects and groups properly
        // if (line_type == "g") {
        //     // new object
        //     regex::regex r_obj_name("g\\s+(\\S+)");
        //     std::string obj_name = "";
        //     if (regex::regex_search(line, match, r_obj_name)) {
        //         obj_name = match[1].str();
        //     }
        //     objects.push_back(Object { obj_name });
        //     current_obj = &objects.back();
        //     continue;
        // }
        if (line_type == "v") {
            auto vert = Vertex::from_line(line);
            if (vert) {
                current_obj->vertices.push_back(*vert);
            }
        }
        // else if (line_type == "vt") {
        //     // ignore this for now
        // }
        // else if (line_type == "vn") {
        //     // ignore this for now
        // }
        else if (line_type == "f") {
            auto face = FaceElement::from_line(line);
            if (face) {
                current_obj->faces.push_back(*face);
            }
        }
    }

    file.close();

    return obj_data;
}

} // namespace obj