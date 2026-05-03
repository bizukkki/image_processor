#pragma once

#include <string>
#include <stdexcept>
#include <vector>

struct Color {
    double r = 0.0;
    double g = 0.0;
    double b = 0.0;
};

class ImageBase {
public:
    ImageBase() {
    }
    ImageBase(int width, int height) : width_(width), height_(height), pixels_(height, std::vector<Color>(width)) {
        if (width < 0 || height < 0) {
            throw std::runtime_error("Невалидные размеры изображения");
        }
    }
    virtual void Read(const std::string& path) = 0;
    virtual void Write(const std::string& path) const = 0;

    int GetWidth() const {
        return width_;
    }
    int GetHeight() const {
        return height_;
    }
    Color& GetPixel(int row, int col) {
        return pixels_[row][col];
    }
    void Resize(int width, int height) {
        if (width <= 0 || height <= 0) {
            throw std::runtime_error("Невалидные размеры изображения");
        }
        int new_width = std::min(width, width_);
        int new_height = std::min(height, height_);

        std::vector<std::vector<Color>> new_pixels;
        new_pixels.resize(new_height);

        for (int row = 0; row < new_height; ++row) {
            new_pixels[row].resize(new_width);
        }
        for (int row = 0; row < new_height; ++row) {
            for (int col = 0; col < new_width; ++col) {
                new_pixels[row][col] = pixels_[row][col];
            }
        }
        pixels_ = new_pixels;
        width_ = new_width;
        height_ = new_height;
    }
    virtual ~ImageBase() = default;

protected:
    int width_ = 0;
    int height_ = 0;
    std::vector<std::vector<Color>> pixels_;
};