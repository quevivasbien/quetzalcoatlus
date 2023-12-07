#include <iostream>
#include <opencv2/opencv.hpp>
#include <OpenImageDenoise/oidn.hpp>

#include "image.hpp"

void Image::save(const std::string& filename, float gamma) const {
    std::vector<float> im_out(width * height * 3);
    // switch to range 0-255 and swap B and R to conform to format expected by opencv
    for (size_t i = 0; i < width * height; i ++) {
        im_out[3 * i + 0] = 255.0f * powf(color_buffer[3 * i + 2], gamma);
        im_out[3 * i + 1] = 255.0f * powf(color_buffer[3 * i + 1], gamma);
        im_out[3 * i + 2] = 255.0f * powf(color_buffer[3 * i + 0], gamma);
    }

    cv::Mat image(height, width, CV_32FC3, im_out.data());

    cv::imwrite(filename, image);
}

void Image::denoise(bool verbose) {
    oidn::DeviceRef device = oidn::newDevice();
    if (verbose) {
        device.set("verbose", 1);
    }
    device.commit();

    oidn::BufferRef colorBuf = device.newBuffer(width * height * 3 * sizeof(float));

    oidn::FilterRef filter = device.newFilter("RT");
    filter.setImage("color", colorBuf, oidn::Format::Float3, width, height);
    filter.setImage("output", colorBuf, oidn::Format::Float3, width, height);
    filter.commit();

    colorBuf.write(0, width * height * 3 * sizeof(float), color_buffer.data());

    filter.execute();
    const char* errorMessage;
    if (device.getError(errorMessage) != oidn::Error::None) {
        std::cerr << "Error while denoising: " << errorMessage << std::endl;
    }

    colorBuf.read(0, width * height * 3 * sizeof(float), color_buffer.data());
}


void RenderResult::denoise(bool verbose) {
    oidn::DeviceRef device = oidn::newDevice();
    if (verbose) {
        device.set("verbose", 1);
    }
    device.commit();

    oidn::BufferRef colorBuf = device.newBuffer(width * height * 3 * sizeof(float));
    oidn::BufferRef normalBuf = device.newBuffer(width * height * 3 * sizeof(float));
    oidn::BufferRef albedoBuf = device.newBuffer(width * height * 3 * sizeof(float));

    colorBuf.write(0, width * height * 3 * sizeof(float), color_buffer.data());
    normalBuf.write(0, width * height * 3 * sizeof(float), normal_buffer.data());
    albedoBuf.write(0, width * height * 3 * sizeof(float), albedo_buffer.data());

    oidn::FilterRef filter = device.newFilter("RT");
    filter.setImage("color", colorBuf, oidn::Format::Float3, width, height);
    filter.setImage("normal", normalBuf, oidn::Format::Float3, width, height);
    filter.setImage("albedo", albedoBuf, oidn::Format::Float3, width, height);
    filter.setImage("output", colorBuf, oidn::Format::Float3, width, height);
    filter.set("cleanAux", true);
    filter.commit();

    // // prefilter normals
    // oidn::FilterRef normalFilter = device.newFilter("RT");
    // normalFilter.setImage("normal", normalBuf, oidn::Format::Float3, width, height);
    // normalFilter.setImage("output", normalBuf, oidn::Format::Float3, width, height);
    // normalFilter.commit();

    // // prefilter albedos
    // oidn::FilterRef albedoFilter = device.newFilter("RT");
    // albedoFilter.setImage("albedo", albedoBuf, oidn::Format::Float3, width, height);
    // albedoFilter.setImage("output", albedoBuf, oidn::Format::Float3, width, height);
    // albedoFilter.commit();

    // normalFilter.execute();
    // albedoFilter.execute();

    filter.execute();

    const char* errorMessage;
    if (device.getError(errorMessage) != oidn::Error::None) {
        std::cerr << "Error while denoising: " << errorMessage << std::endl;
    }

    colorBuf.read(0, width * height * 3 * sizeof(float), color_buffer.data());
    normalBuf.read(0, width * height * 3 * sizeof(float), normal_buffer.data());
    albedoBuf.read(0, width * height * 3 * sizeof(float), albedo_buffer.data());
}

void RenderResult::save_normal(const std::string& filename) const {
    Image normal_im(albedo_buffer, height, width);
    normal_im.save(filename);
}

void RenderResult::save_albedo(const std::string& filename) const {
    Image albedo_im(albedo_buffer, height, width);
    albedo_im.save(filename);
}
