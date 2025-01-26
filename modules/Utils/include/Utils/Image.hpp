///
/// @file Image.hpp
/// @brief This file contains the image class
/// @namespace ven
///

#pragma once

namespace ven {

	using pixel = unsigned char;

	///
	/// @class Image
	/// @brief Class for image
	/// @namespace ven
	///
	class Image {

    	public:

        	explicit Image(const std::string &path);
        	~Image();

            pixel* pixels = nullptr;
            int width = 0;
            int height = 0;
            int channels = 0;

	}; // class Image

} // namespace ven
