///
/// @file Mesh.hpp
/// @brief This file contains the mesh class
/// @namespace ven
///

#pragma once

#include <memory>

#include "VEngine/Gfx/Resources/Vertex.hpp"
#include "VEngine/Gfx/Resources/Texture.hpp"

namespace ven {

    class Mesh {

        public:

            Mesh() = default;
            ~Mesh() = default;

            Mesh(const Mesh&) = delete;
            Mesh& operator=(const Mesh&) = delete;
            Mesh(Mesh&&) = delete;
            Mesh& operator=(Mesh&&) = delete;

            void addVertex(const Vertex& vertex) { vertices.push_back(vertex); }
            void addIndex(const uint32_t index) { indices.push_back(index); }
            void setTexture(const std::shared_ptr<Texture>& texture) { m_texture = texture; }

            [[nodiscard]] const std::shared_ptr<Texture>& getTexture() const { return m_texture; }
            [[nodiscard]] const std::vector<Vertex>& getVertices() const { return vertices; }
            [[nodiscard]] const std::vector<uint32_t>& getIndices() const { return indices; }

        private:

            std::shared_ptr<Texture> m_texture = nullptr;
            std::vector<Vertex> vertices;
            std::vector<uint32_t> indices;

    };

} // namespace ven
