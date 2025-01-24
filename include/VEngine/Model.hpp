///
/// @file Model.hpp
/// @brief This file contains the model class
/// @namespace ven
///

#pragma once

#include <vector>

#include "VEngine/Vertex.hpp"

namespace ven {

    ///
    /// @class Model
    /// @brief Class for models
    /// @namespace ven
    ///
    class Model {

        public:

        explicit Model(const std::string &path);

        ~Model() = default;

        Model(const Model&) = delete;
        Model& operator=(const Model&) = delete;
        Model(Model&&) = delete;
        Model& operator=(Model&&) = delete;


        [[nodiscard]] const std::vector<Vertex>& getVertices() const { return vertices; }
        [[nodiscard]] const std::vector<uint32_t>& getIndices() const { return indices; }

        private:

        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;

    }; // class Model

} // namespace ven
