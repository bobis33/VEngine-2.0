#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

#include "VEngine/Gfx/Resources/Model.hpp"
#include "VEngine/Gfx/Resources/TextureManager.hpp"

template<> struct std::hash<ven::Vertex> {
    size_t operator()(ven::Vertex const& vertex) const noexcept {
        return ((hash<glm::vec3>()(vertex.pos) ^ (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^ (hash<glm::vec2>()(vertex.texCoord) << 1);
    }
};

ven::Model::Model(const Device& device, const SwapChain& swapChain, const std::string& path)
    : m_device(device), m_swapChain(swapChain) {
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices);
    if (scene == nullptr || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) != 0U || scene->mRootNode == nullptr) {
        throw utl::THROW_ERROR(importer.GetErrorString());
    }
    processNode(scene->mRootNode, scene, device, swapChain);
}

void ven::Model::processNode(const aiNode* node, const aiScene* scene, const Device& device, const SwapChain& swapChain) {
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        const aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        m_meshes.push_back(processMesh(mesh, scene, device, swapChain));
    }
    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        processNode(node->mChildren[i], scene, device, swapChain);
    }
}

std::unique_ptr<ven::Mesh> ven::Model::processMesh(const aiMesh* mesh, const aiScene* scene, const Device& device, const SwapChain& swapChain) {
    auto newMesh = std::make_unique<Mesh>();
    std::unordered_map<Vertex, uint32_t> uniqueVertices{};
    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        Vertex vertex{};
        vertex.pos = {
            mesh->mVertices[i].x,
            mesh->mVertices[i].y,
            mesh->mVertices[i].z
        };
        vertex.color = {1.0F, 1.0F, 1.0F};
        if (mesh->mTextureCoords[0] != nullptr) {
            vertex.texCoord = {
                mesh->mTextureCoords[0][i].x,
                1.0F - mesh->mTextureCoords[0][i].y
            };
        } else {
            vertex.texCoord = {0.0F, 0.0F};
        }
        if (!uniqueVertices.contains(vertex)) {
            uniqueVertices[vertex] = static_cast<uint32_t>(newMesh->getVertices().size());
            newMesh->addVertex(vertex);
        }
    }
    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        const aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++) {
            newMesh->addIndex(uniqueVertices[Vertex{.pos = {
                mesh->mVertices[face.mIndices[j]].x,
                mesh->mVertices[face.mIndices[j]].y,
                mesh->mVertices[face.mIndices[j]].z
            }, .color = {1.0F, 1.0F, 1.0F}, .texCoord = (mesh->mTextureCoords[0] != nullptr) ? glm::vec2{
                mesh->mTextureCoords[0][face.mIndices[j]].x,
                1.0F - mesh->mTextureCoords[0][face.mIndices[j]].y
            } : glm::vec2{0.0F, 0.0F}
            }]);
        }
    }
    if (mesh->mMaterialIndex >= 0) {
        const aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        aiString texturePath;
        if (material->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
            material->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath);
            newMesh->setTexture(TextureManager::getTexture(device, swapChain, "assets/textures/" + std::string(texturePath.C_Str())));
        } else {
            newMesh->setTexture(TextureManager::getTexture(device, swapChain, "assets/textures/default.png"));
        }
    }
    return newMesh;
}
