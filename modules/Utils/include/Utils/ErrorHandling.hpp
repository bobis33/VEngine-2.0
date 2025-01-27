#pragma once

#include <stdexcept>
#include <string>

#define THROW_ERROR(msg) throw std::runtime_error(std::string(msg) + " (" + __FILE__ + ":" + std::to_string(__LINE__) + ")")
