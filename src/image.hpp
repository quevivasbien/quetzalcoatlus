#pragma once

#include <cstddef>
#include <vector>
#include <string>

class Image {
public:
    Image(std::vector<float>&& buffer, size_t height, size_t width) : width(width), height(height), color_buffer(buffer) {}

    void save(const std::string& filename) const;

    size_t height;
    size_t width;
    std::vector<float> color_buffer;
};

class RenderResult : public Image {
public:
    RenderResult(std::vector<float>&& buffer, size_t height, size_t width) : Image(std::move(buffer), height, width) {}

    void denoise();
};
