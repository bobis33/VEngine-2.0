#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include "VEngine/Gfx/Model.hpp"

template<> struct std::hash<ven::Vertex> {
    size_t operator()(ven::Vertex const& vertex) const noexcept {
        return ((hash<glm::vec3>()(vertex.pos) ^ (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^ (hash<glm::vec2>()(vertex.texCoord) << 1);
    }
};

ven::Model::Model(const std::string &path) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn;
    std::string err;
    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str())) {
        throw std::runtime_error(warn + err);
    }
    std::unordered_map<Vertex, uint32_t> uniqueVertices{};
    for (const auto& shape : shapes) {
        for (const auto& index : shape.mesh.indices) {
            Vertex vertex{};
            vertex.pos = {
                attrib.vertices[(3 * index.vertex_index) + 0],
                attrib.vertices[(3 * index.vertex_index) + 1],
                attrib.vertices[(3 * index.vertex_index) + 2]
            };
            vertex.texCoord = {
                attrib.texcoords[(2 * index.texcoord_index) + 0],
                1.0F - attrib.texcoords[(2 * index.texcoord_index) + 1]
            };
            vertex.color = {1.0F, 1.0F, 1.0F};
            if (!uniqueVertices.contains(vertex)) {
                uniqueVertices[vertex] = static_cast<uint32_t>(m_vertices.size());
                m_vertices.push_back(vertex);
            }
            m_indices.push_back(uniqueVertices[vertex]);
        }
    }
}
