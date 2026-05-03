#pragma once

#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "filters.h"

class Parser {
public:
    Parser(int argc, char** argv) {
        Parse(argc, argv);
    }
    std::string& GetInputPath() {
        return input_path_;
    }
    std::string& GetOutputPath() {
        return output_path_;
    }
    std::vector<std::unique_ptr<Filter>>& GetFilters() {
        return filters_;
    }

private:
    std::string input_path_;
    std::string output_path_;
    std::vector<std::unique_ptr<Filter>> filters_;

    void Parse(int argc, char** argv) {
        if (argc < 3) {
            throw std::runtime_error("Ожидаеммые данные: image_processorinput.bmp output.bmp [filters]\n");
        }

        input_path_ = argv[1];
        output_path_ = argv[2];

        for (int i = 3; i < argc; ++i) {
            std::string arg = argv[i];
            if (arg == "-neg") {
                filters_.push_back(std::make_unique<NegativeFilter>());

            } else if (arg == "-crop") {
                if (i + 2 >= argc) {
                    throw std::runtime_error("crop ожидает два параметра");
                }

                int width = std::stoi(argv[i + 1]);
                int height = std::stoi(argv[i + 2]);
                filters_.push_back(std::make_unique<CropFilter>(width, height));
                i += 2;
            } else if (arg == "-gs") {
                filters_.push_back(std::make_unique<GrayscaleFilter>());
            } else if (arg == "-sharp") {
                filters_.push_back(std::make_unique<SharpeningFilter>());
            } else if (arg == "-edge") {
                if (i + 1 >= argc) {
                    throw std::runtime_error("edge ожидает один параметр");
                }
                double threshold = std::stod(argv[i + 1]);
                filters_.push_back(std::make_unique<EdgeDetectionFilter>(threshold));
                i += 1;
            } else if (arg == "-blur") {
                if (i + 1 >= argc) {
                    throw std::runtime_error("blur ожидает один параметр");
                }
                double sigma = std::stod(argv[i + 1]);
                filters_.push_back(std::make_unique<BlurFilter>(sigma));
                i += 1;
            } else if (arg == "-pixelate") {
                if (i + 1 >= argc) {
                    throw std::runtime_error("pixelate ожидает один параметр");
                }
                double scale = std::stod(argv[i + 1]);
                filters_.push_back(std::make_unique<PixelateFilter>(scale));
                i += 1;
            } else {
                throw std::runtime_error("Неизвестный фильтр");
            }
        }
    }
};