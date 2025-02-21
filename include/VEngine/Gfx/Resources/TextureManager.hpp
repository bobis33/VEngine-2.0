///
/// @file TextureManager.hpp
/// @brief This file contains the TextureManager class
/// @namespace ven
///

#pragma once

#include "VEngine/Gfx/Resources/Texture.hpp"

namespace ven {
    ///
    /// @class TextureManager
    /// @brief Class for texture manager
    /// @namespace ven
    ///
    class TextureManager {

        public:

            [[nodiscard]] static std::shared_ptr<Texture> getTexture(const Device& device, const SwapChain& swapChain, const std::string& filepath) {
                auto& instance = getInstance();
                if (instance.m_texturePaths.contains(filepath)) {
                    const uint8_t index = instance.m_texturePaths[filepath];
                    return instance.m_textures[index];
                }
                const uint8_t newIndex = instance.m_nextIndex++;
                auto texture = std::make_shared<Texture>(device, swapChain, filepath);
                instance.m_textures[newIndex] = texture;
                instance.m_texturePaths[filepath] = newIndex;
                return texture;
            }

            [[nodiscard]] static const std::shared_ptr<Texture>& getTexturePath(const uint8_t index) {
                if (auto& instance = getInstance(); instance.m_textures.contains(index)) {
                    return instance.m_textures[index];
                }
                throw utl::THROW_ERROR(("Texture not found with index: " + std::to_string(index)).c_str());
            }

            [[nodiscard]] static uint8_t getTextureIndex(const std::string& filepath) {
                if (auto& instance = getInstance(); instance.m_texturePaths.contains(filepath)) {
                    return instance.m_texturePaths[filepath];
                }
                throw utl::THROW_ERROR(("Texture not found with path: " + filepath).c_str());
            }

            static void clean() {
                for (const auto& [key, texture] : getInstance().m_textures) {
                    texture.get()->clean();
                }
            }

            [[nodiscard]] static const std::unordered_map<uint8_t, std::shared_ptr<Texture>>& getTextures() { return getInstance().m_textures; }
            [[nodiscard]] static size_t getTextureSize() { return getInstance().m_textures.size(); }

        private:

            uint8_t m_nextIndex = 0;
            std::unordered_map<uint8_t, std::shared_ptr<Texture>> m_textures;
            std::unordered_map<std::string, uint8_t> m_texturePaths;

            [[nodiscard]] static TextureManager& getInstance() {
                static TextureManager instance;
                return instance;
            }

    }; // class TextureManager

} // namespace ven