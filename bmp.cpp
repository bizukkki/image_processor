#include "bmp.h"

#include <fstream>

const uint16_t BMP_SIGNATURE = 0x4D42;
const uint16_t BMP_BITS_PER_PIXEL = 24;
const int BYTES_PER_PIXEL = 3;
const double COLOR_SCALE = 255.0;

static int GetPadding(int width) {
    return (4 - (width * BYTES_PER_PIXEL) % 4) % 4;
}

static unsigned char ToByte(double value) {
    value = std::max(0.0, std::min(1.0, value));
    return static_cast<unsigned char>(value * COLOR_SCALE);
}

void ImageBMP::Read(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Ошибка: не удалось открыть файл!");
    }
    BMPFileHeader file_header{};
    BMPInfoHeader info_header{};

    file.read(reinterpret_cast<char*>(&file_header), sizeof(file_header));
    file.read(reinterpret_cast<char*>(&info_header), sizeof(info_header));

    if (file_header.signature != BMP_SIGNATURE) {
        throw std::runtime_error("Ошибка: это не BMP файл!");
    }

    if (info_header.bitsPerPixel != BMP_BITS_PER_PIXEL) {
        throw std::runtime_error("Ошибка: возможен только 24-битный BMP!");
    }
    if (info_header.compression != 0) {
        throw std::runtime_error("Ошибка: файл должен быть без сжатия!");
    }
    width_ = info_header.width;
    height_ = info_header.height;
    pixels_ = std::vector<std::vector<Color>>(height_, std::vector<Color>(width_));

    const int padding = GetPadding(width_);
    file.seekg(file_header.dataOffset, std::ios::beg);

    for (int row = height_ - 1; row >= 0; --row) {
        for (int col = 0; col < width_; ++col) {
            unsigned char blue = 0;
            unsigned char green = 0;
            unsigned char red = 0;

            file.read(reinterpret_cast<char*>(&blue), 1);
            file.read(reinterpret_cast<char*>(&green), 1);
            file.read(reinterpret_cast<char*>(&red), 1);

            pixels_[row][col] = {red / COLOR_SCALE, green / COLOR_SCALE, blue / COLOR_SCALE};
        }
        file.ignore(padding);
    }
}

void ImageBMP::Write(const std::string& path) const {
    std::ofstream file(path, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Ошибка: не удалось открыть файл!");
    }

    const int padding = GetPadding(width_);
    const uint32_t row_size = width_ * BYTES_PER_PIXEL + padding;
    const uint32_t image_size = row_size * height_;
    const uint32_t headers_size = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader);

    BMPFileHeader file_header{};
    BMPInfoHeader info_header{};

    file_header.signature = BMP_SIGNATURE;
    file_header.fileSize = headers_size + image_size;
    file_header.reserved = 0;
    file_header.dataOffset = headers_size;

    info_header.headerSize = sizeof(BMPInfoHeader);
    info_header.width = width_;
    info_header.height = height_;
    info_header.planes = 1;
    info_header.bitsPerPixel = BMP_BITS_PER_PIXEL;
    info_header.compression = 0;
    info_header.imageSize = image_size;
    info_header.xPixelsPerM = 0;
    info_header.yPixelsPerM = 0;
    info_header.colorsUsed = 0;
    info_header.colorsImportant = 0;

    file.write(reinterpret_cast<const char*>(&file_header), sizeof(file_header));
    file.write(reinterpret_cast<const char*>(&info_header), sizeof(info_header));

    for (int row = height_ - 1; row >= 0; --row) {
        for (int col = 0; col < width_; ++col) {
            const Color& pixel = pixels_[row][col];

            unsigned char red = ToByte(pixel.r);
            unsigned char green = ToByte(pixel.g);
            unsigned char blue = ToByte(pixel.b);

            file.put(static_cast<char>(blue));
            file.put(static_cast<char>(green));
            file.put(static_cast<char>(red));
        }

        for (int i = 0; i < padding; ++i) {
            file.put(0);
        }
    }
}