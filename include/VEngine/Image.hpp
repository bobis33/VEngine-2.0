#pragma once

namespace ven {

	class Image {

        using pixel = unsigned char;

    	public:

        	explicit Image(const std::string &path);
        	~Image();

            pixel* pixels = nullptr;
            int width = 0;
            int height = 0;
            int channels = 0;

	}; // class Image

}; // namespace ven