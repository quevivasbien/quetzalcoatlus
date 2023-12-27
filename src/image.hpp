#pragma once

#include <cstddef>
#include <vector>
#include <string>

class Image {
public:
    Image(size_t height, size_t width) : height(height), width(width), color_buffer(height * width * 3) {}

    Image(const std::vector<float>& buffer, size_t height, size_t width) : height(height), width(width), color_buffer(buffer) {}
    Image(std::vector<float>&& buffer, size_t height, size_t width) : height(height), width(width), color_buffer(buffer) {}

    void save(const std::string& filename, float gamma = 1.0) const;

    virtual void denoise(bool verbose = false);

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

    virtual void denoise(bool verbose = false) override;

    void save_normal(const std::string& filename) const;
    void save_albedo(const std::string& filename) const;

    std::vector<float> normal_buffer;
    std::vector<float> albedo_buffer;
};
