#include "VEngine/Engine.hpp"

int main() {
    try {
        Engine app;
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}