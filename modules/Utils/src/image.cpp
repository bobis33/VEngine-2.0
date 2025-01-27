#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "Utils/ErrorHandling.hpp"
#include "Utils/Image.hpp"

utl::Image::Image(const std::string &path)
{
    pixels = stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);
    if (pixels == nullptr) {
        THROW_ERROR(("failed to load image: " + path).c_str());
    }
}

utl::Image::~Image() { stbi_image_free(pixels); }