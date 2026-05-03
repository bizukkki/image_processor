#pragma once

#include <vector>
#include <stdexcept>
#include <cmath>
#include "image.h"

class Filter {
public:
    virtual ~Filter() {
    }
    virtual void Apply(ImageBase& image) = 0;
};

class NegativeFilter : public Filter {
public:
    void Apply(ImageBase& image) override {
        for (int row = 0; row < image.GetHeight(); ++row) {
            for (int col = 0; col < image.GetWidth(); ++col) {
                Color& pixel = image.GetPixel(row, col);
                pixel.r = 1.0 - pixel.r;
                pixel.g = 1.0 - pixel.g;
                pixel.b = 1.0 - pixel.b;
            }
        }
    }
};

class GrayscaleFilter : public Filter {
public:
    void Apply(ImageBase& image) override {
        const double red_weight = 0.299;
        const double green_weight = 0.587;
        const double blue_weight = 0.114;
        for (int row = 0; row < image.GetHeight(); ++row) {
            for (int col = 0; col < image.GetWidth(); ++col) {
                Color& pixel = image.GetPixel(row, col);
                double new_color = pixel.r * red_weight + pixel.g * green_weight + pixel.b * blue_weight;
                pixel.r = new_color;
                pixel.g = new_color;
                pixel.b = new_color;
            }
        }
    }
};

class MatrixFilter : public Filter {
protected:
    explicit MatrixFilter(const std::vector<std::vector<double>>& matrix) : matrix_(matrix) {
    }
    std::vector<std::vector<double>> matrix_;
    double NormalizeValue(double value) {
        return std::max(0.0, std::min(1.0, value));
    }
    int NormalizeIndex(int value, int min_value, int max_value) {
        return std::max(min_value, std::min(max_value, value));
    }

    void ApplyMatrix(ImageBase& image) {
        int height = image.GetHeight();
        int width = image.GetWidth();
        std::vector<std::vector<Color>> old_pixels(height, std::vector<Color>(width));
        for (int row = 0; row < height; ++row) {
            for (int col = 0; col < width; ++col) {
                old_pixels[row][col] = image.GetPixel(row, col);
            }
        }
        for (int row = 0; row < height; ++row) {
            for (int col = 0; col < width; ++col) {
                Color result;
                for (int y = -1; y <= 1; ++y) {
                    for (int x = -1; x <= 1; ++x) {
                        int source_row = NormalizeIndex(row + y, 0, height - 1);
                        int source_col = NormalizeIndex(col + x, 0, width - 1);
                        double coefficient = matrix_[y + 1][x + 1];
                        Color& pixel = old_pixels[source_row][source_col];
                        result.r += pixel.r * coefficient;
                        result.g += pixel.g * coefficient;
                        result.b += pixel.b * coefficient;
                    }
                }
                result.r = NormalizeValue(result.r);
                result.g = NormalizeValue(result.g);
                result.b = NormalizeValue(result.b);
                image.GetPixel(row, col) = result;
            }
        }
    }
};

class SharpeningFilter : public MatrixFilter {
public:
    SharpeningFilter() : MatrixFilter({{0.0, -1.0, 0.0}, {-1.0, Center, -1.0}, {0.0, -1.0, 0.0}}) {
    }
    void Apply(ImageBase& image) override {
        ApplyMatrix(image);
    }

private:
    static constexpr double Center = 5.0;
};

class EdgeDetectionFilter : public MatrixFilter {
public:
    explicit EdgeDetectionFilter(double threshold)
        : MatrixFilter({{0.0, -1.0, 0.0}, {-1.0, Center, -1.0}, {0.0, -1.0, 0.0}}), threshold_(threshold) {
        if (threshold < 0.0 || threshold > 1.0) {
            throw std::runtime_error("threshold должен быть от 0 до 1");
        }
    }

    void Apply(ImageBase& image) override {
        GrayscaleFilter grayscale;
        grayscale.Apply(image);
        ApplyMatrix(image);
        for (int row = 0; row < image.GetHeight(); ++row) {
            for (int col = 0; col < image.GetWidth(); ++col) {
                Color& pixel = image.GetPixel(row, col);
                if (pixel.r > threshold_) {
                    pixel.r = 1.0;
                    pixel.g = 1.0;
                    pixel.b = 1.0;
                } else {
                    pixel.r = 0.0;
                    pixel.g = 0.0;
                    pixel.b = 0.0;
                }
            }
        }
    }

private:
    double threshold_;
    static constexpr double Center = 4.0;
};

class CropFilter : public Filter {
public:
    CropFilter(int width, int height) : width_(width), height_(height) {
        if (width <= 0 || height <= 0) {
            throw std::runtime_error("Невалидные значения для crop");
        }
    }
    void Apply(ImageBase& image) override {
        image.Resize(width_, height_);
    }

private:
    int width_;
    int height_;
};

class BlurFilter : public Filter {
public:
    explicit BlurFilter(double sigma) : sigma_(sigma) {
    }
    void Apply(ImageBase& image) override {
        int height = image.GetHeight();
        int width = image.GetWidth();
        std::vector<double> kernel = BuildKernel(sigma_);
        int radius = static_cast<int>(kernel.size()) / 2;

        std::vector<std::vector<Color>> old_pixels(height, std::vector<Color>(width));
        for (int row = 0; row < height; ++row) {
            for (int col = 0; col < width; ++col) {
                old_pixels[row][col] = image.GetPixel(row, col);
            }
        }
        std::vector<std::vector<Color>> temp_pixels(height, std::vector<Color>(width));

        for (int row = 0; row < height; ++row) {
            for (int col = 0; col < width; ++col) {
                Color result;
                for (int x = -radius; x <= radius; ++x) {
                    int source_col = NormalizeIndex(col + x, 0, width - 1);
                    double coefficient = kernel[x + radius];
                    const Color& pixel = old_pixels[row][source_col];
                    result.r += pixel.r * coefficient;
                    result.g += pixel.g * coefficient;
                    result.b += pixel.b * coefficient;
                }
                temp_pixels[row][col] = result;
            }
        }
        for (int row = 0; row < height; ++row) {
            for (int col = 0; col < width; ++col) {
                Color result;
                for (int y = -radius; y <= radius; ++y) {
                    int source_row = NormalizeIndex(row + y, 0, height - 1);
                    double coefficient = kernel[y + radius];
                    Color& pixel = temp_pixels[source_row][col];
                    result.r += pixel.r * coefficient;
                    result.g += pixel.g * coefficient;
                    result.b += pixel.b * coefficient;
                }
                result.r = NormalizeValue(result.r);
                result.g = NormalizeValue(result.g);
                result.b = NormalizeValue(result.b);
                image.GetPixel(row, col) = result;
            }
        }
    }

private:
    static std::vector<double> BuildKernel(double sigma) {
        if (sigma <= 0.0) {
            throw std::runtime_error("sigma должен быть больше 0");
        }
        const double multiplier = 3.0;
        const double two = 2.0;
        int radius = std::ceil(multiplier * sigma);
        int size = 2 * radius + 1;
        std::vector<double> kernel(size);
        double sum = 0.0;
        for (int i = -radius; i <= radius; ++i) {
            double value = (1.0 / std::sqrt(two * M_PI * sigma * sigma)) * std::exp(-(i * i) / (two * sigma * sigma));
            kernel[i + radius] = value;
            sum += value;
        }
        for (int i = 0; i < size; ++i) {
            kernel[i] /= sum;
        }
        return kernel;
    }

    double NormalizeValue(double value) {
        return std::max(0.0, std::min(1.0, value));
    }

    int NormalizeIndex(int value, int min_value, int max_value) {
        return std::max(min_value, std::min(max_value, value));
    }

    double sigma_;
};

class PixelateFilter : public Filter {
public:
    explicit PixelateFilter(int scale) : scale_(scale) {
        if (scale <= 0) {
            throw std::runtime_error("scale должен быть больше 0");
        }
    }
    void Apply(ImageBase& image) override {
        int height = image.GetHeight();
        int width = image.GetWidth();
        std::vector<std::vector<Color>> old_pixels(height, std::vector<Color>(width));
        for (int row = 0; row < height; ++row) {
            for (int col = 0; col < width; ++col) {
                old_pixels[row][col] = image.GetPixel(row, col);
            }
        }
        for (int start_row = 0; start_row < height; start_row += scale_) {
            for (int start_col = 0; start_col < width; start_col += scale_) {
                int end_row = std::min(start_row + scale_, height);
                int end_col = std::min(start_col + scale_, width);
                Color mid_color;
                int pixel_count = 0;
                for (int row = start_row; row < end_row; ++row) {
                    for (int col = start_col; col < end_col; ++col) {
                        mid_color.r += old_pixels[row][col].r;
                        mid_color.g += old_pixels[row][col].g;
                        mid_color.b += old_pixels[row][col].b;
                        ++pixel_count;
                    }
                }
                mid_color.r /= pixel_count;
                mid_color.g /= pixel_count;
                mid_color.b /= pixel_count;
                for (int row = start_row; row < end_row; ++row) {
                    for (int col = start_col; col < end_col; ++col) {
                        image.GetPixel(row, col) = mid_color;
                    }
                }
            }
        }
    }

private:
    int scale_;
};