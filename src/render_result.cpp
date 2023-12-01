#include <iostream>
#include <opencv2/opencv.hpp>
#include <OpenImageDenoise/oidn.hpp>

#include "render_result.hpp"

void RenderResult::save(const std::string& filename) const {
    std::vector<float> im_out(width * height * 3);
    std::transform(buffer.begin(), buffer.end(), im_out.begin(), [](float c) { return c * 255.0f; });

    cv::Mat image(height, width, CV_32FC3, im_out.data());
    cv::imwrite(filename, image);
}

void RenderResult::denoise() {
    oidn::DeviceRef device = oidn::newDevice();
    device.set("verbose", 1);
    device.commit();

    oidn::BufferRef colorBuf = device.newBuffer(width * height * 3 * sizeof(float));

    oidn::FilterRef filter = device.newFilter("RT");
    filter.setImage("color", colorBuf, oidn::Format::Float3, width, height);
    filter.setImage("output", colorBuf, oidn::Format::Float3, width, height);
    filter.commit();

    colorBuf.write(0, width * height * 3 * sizeof(float), buffer.data());

    filter.execute();
    const char* errorMessage;
    if (device.getError(errorMessage) != oidn::Error::None) {
        std::cerr << "Error while denoising: " << errorMessage << std::endl;
    }

    colorBuf.read(0, width * height * 3 * sizeof(float), buffer.data());
}
