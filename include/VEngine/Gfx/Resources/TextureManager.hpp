///
/// @file TextureManager.hpp
/// @brief This file contains the TextureManager class
/// @namespace ven
///

#pragma once

#include <memory>

#include "VEngine/Gfx/Resources/Texture.hpp"

namespace ven {
    ///
    /// @class TextureManager
    /// @brief Class for texture manager
    /// @namespace ven
    ///
    class TextureManager {

        public:

            static std::shared_ptr<Texture> getTexture(const Device& device, const SwapChain& swapChain, const std::string& filepath) {
                auto& instance = getInstance();
                if (instance.m_texturePaths.contains(filepath)) {
                    const int index = instance.m_texturePaths[filepath];
                    return instance.m_textures[index];
                }
                const int newIndex = instance.m_nextIndex++;
                auto texture = std::make_shared<Texture>(device, swapChain, filepath);
                instance.m_textures[newIndex] = texture;
                instance.m_texturePaths[filepath] = newIndex;
                return texture;
            }

            static std::shared_ptr<Texture> getTexturePath(const int index) {
                if (auto& instance = getInstance(); instance.m_textures.contains(index)) {
                    return instance.m_textures[index];
                }
                return nullptr;
            }

            static int getTextureIndex(const std::string& filepath) {
                if (auto& instance = getInstance(); instance.m_texturePaths.contains(filepath)) {
                    return instance.m_texturePaths[filepath];
                }
                return -1;
            }

            static void clean() {
                for (const auto& [key, texture] : getInstance().m_textures) {
                    texture.get()->clean();
                }
            }

        private:

            int m_nextIndex = 0;
            std::unordered_map<int, std::shared_ptr<Texture>> m_textures;
            std::unordered_map<std::string, int> m_texturePaths;

            static TextureManager& getInstance() {
                static TextureManager instance;
                return instance;
            }

    }; // class TextureManager

} // namespace ven