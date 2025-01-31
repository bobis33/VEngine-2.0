#include <iostream>

#include "VEngine/Engine.hpp"

int main() {
    try {
        ven::Engine().run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
