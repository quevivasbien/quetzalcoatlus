#include <cstddef>
#include <vector>
#include <string>

class RenderResult {
public:
    RenderResult(std::vector<float>&& buffer, size_t height, size_t width) : width(width), height(height), buffer(buffer) {}

    void save(const std::string& filename) const;
    void denoise();

    size_t height;
    size_t width;
    std::vector<float> buffer;
};
