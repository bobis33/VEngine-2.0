///
/// @file Utils.hpp
/// @brief This file contains utility functions
/// @namespace utl
///

#pragma once

#include <fstream>
#include <vector>

#include "Utils/ErrorHandling.hpp"

namespace utl {

     inline std::vector<char> readFile(const std::string& filename) {
          std::ifstream file(filename, std::ios::binary | std::ios::ate);
          if (!file.is_open()) {
               throw THROW_ERROR(("failed to open file " + filename).c_str());
          }
          const size_t fileSize = file.tellg();
          if (fileSize <= 0) {
               throw THROW_ERROR(("file " + filename + " is empty").c_str());
          }
          std::vector<char> buffer(fileSize);
          file.seekg(0, std::ios::beg);
          if (!file.read(buffer.data(), fileSize)) {
               throw THROW_ERROR(("failed to read file " + filename).c_str());
          }
          return buffer;
     }

} // namespace utl
