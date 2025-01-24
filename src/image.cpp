#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "VEngine/Image.hpp"

ven::Image::Image(const std::string &path)
{
    pixels = stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);
    if (!pixels) {
        std::cerr << "Failed to load texture image at: " << path << '\n';
    }
}

ven::Image::~Image()
{
    stbi_image_free(pixels);
}