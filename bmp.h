#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <stdexcept>

#include "image.h"

#pragma pack(push, 1)
struct BMPFileHeader {
    uint16_t signature;
    uint32_t fileSize;
    uint32_t reserved;
    uint32_t dataOffset;
};

struct BMPInfoHeader {
    uint32_t headerSize;
    int32_t width;
    int32_t height;
    uint16_t planes;
    uint16_t bitsPerPixel;
    uint32_t compression;
    uint32_t imageSize;
    int32_t xPixelsPerM;
    int32_t yPixelsPerM;
    uint32_t colorsUsed;
    uint32_t colorsImportant;
};
#pragma pack(pop)

class ImageBMP : public ImageBase {
public:
    void Read(const std::string& path) override;
    void Write(const std::string& path) const override;
};