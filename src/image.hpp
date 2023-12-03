#pragma once

#include <cstddef>
#include <vector>
#include <string>

class Image {
public:
    Image(size_t height, size_t width) : height(height), width(width), color_buffer(height * width * 3) {}

    Image(std::vector<float>&& buffer, size_t height, size_t width) : width(width), height(height), color_buffer(buffer) {}

    void save(const std::string& filename, float gamma = 0.43) const;

    virtual void denoise();

    size_t height;
    size_t width;
    std::vector<float> color_buffer;
};

class RenderResult : public Image {
public:
    RenderResult(size_t height, size_t width) : Image(height, width), normal_buffer(height * width * 3), albedo_buffer(height * width * 3) {}

    RenderResult(
        std::vector<float>&& color_buffer,
        std::vector<float>&& normal_buffer,
        std::vector<float>&& albedo_buffer,
        size_t height,
        size_t width
    ) : Image(std::move(color_buffer), height, width) {}

    virtual void denoise() override;

    std::vector<float> normal_buffer;
    std::vector<float> albedo_buffer;
};
