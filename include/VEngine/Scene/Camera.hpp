///
/// @file Camera.hpp
/// @brief This file contains the Camera class
/// @namespace ven
///

#pragma once

#include <glm/glm.hpp>

namespace ven {

    ///
    /// @class Camera
    /// @brief Class for camera
    /// @namespace ven
    ///
    class Camera {

        public:

            Camera() = default;
            ~Camera() = default;

            Camera(const Camera &) = delete;
            Camera& operator=(const Camera &) = delete;
            Camera(Camera &&) = delete;
            Camera& operator=(Camera &&) = delete;

            void move(const glm::vec3& direction, const float deltaTime);
            void rotate(const float yawOffset, const float pitchOffset, const float deltaTime);
            [[nodiscard]] glm::mat4 getProjectionMatrix(float aspectRatio) const;
            [[nodiscard]] glm::mat4 getViewMatrix() const;

            [[nodiscard]] glm::vec3 getPosition() const { return m_position; }
            [[nodiscard]] glm::vec3 getFront() const { return m_front; }
            [[nodiscard]] glm::vec3 getUp() const { return m_up; }
            [[nodiscard]] glm::vec3 getRight() const { return m_right; }
            [[nodiscard]] float getFov() const { return m_fov; }
            [[nodiscard]] float getNear() const { return m_near; }
            [[nodiscard]] float getFar() const { return m_far; }
            [[nodiscard]] float getMoveSpeed() const { return m_moveSpeed; }
            [[nodiscard]] float getLookSpeed() const { return m_lookSpeed; }

        private:

            glm::vec3 m_position = glm::vec3(0.0F, 0.0F, 3.0F);
            glm::vec3 m_front = glm::vec3(0.0F, 0.0F, -1.0F);
            glm::vec3 m_up = glm::vec3(0.0F, 1.0F, 0.0F);
            glm::vec3 m_right = glm::vec3(1.0F, 0.0F, 0.0F);
            glm::vec3 m_worldUp = glm::vec3(0.0F, 1.0F, 0.0F);
            float m_moveSpeed = 2.5F;
            float m_lookSpeed = 100.0F;
            float m_fov = 45.0F;
            float m_near = 0.1F;
            float m_far = 100.0F;
            float m_yaw = -90.0F;
            float m_pitch = 1.0F;

    }; // class Camera

} // namespace ven
