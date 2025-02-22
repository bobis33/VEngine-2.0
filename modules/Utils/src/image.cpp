#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "Utils/ErrorHandling.hpp"
#include "Utils/Image.hpp"

utl::Image::Image(const std::string& path, const int flip) {
    stbi_set_flip_vertically_on_load(flip);
    pixels = stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);
    if (pixels == nullptr) {
        throw THROW_ERROR(("failed to load image: " + path).c_str());
    }
}

utl::Image::~Image() { stbi_image_free(pixels); }
