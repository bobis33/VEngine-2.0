#include <stdexcept>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "Utils/Image.hpp"

ven::Image::Image(const std::string &path)
{
    pixels = stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);
    if (pixels == nullptr) {
        throw std::runtime_error("failed to load image: " + path);
    }
}

ven::Image::~Image()
{
    stbi_image_free(pixels);
}
