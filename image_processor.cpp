#include "bmp.h"
#include "parser.h"

int main(int argc, char** argv) {
    Parser parser(argc, argv);
    ImageBMP image;
    image.Read(parser.GetInputPath());
    for (auto& filter : parser.GetFilters()) {
        filter->Apply(image);
    }
    image.Write(parser.GetOutputPath());
}