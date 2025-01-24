///
/// @file Clock.hpp
/// @brief This file contains the Clock class
/// @namespace utl
///

#pragma once

#include <chrono>

namespace utl {

    using TimePoint = std::chrono::time_point<std::chrono::high_resolution_clock>;

    ///
    /// @class Clock
    /// @brief Class for clock
    /// @namespace utl
    ///
    class Clock {

        public:

            Clock() { start(); }
            ~Clock() = default;

            Clock(const Clock&) = delete;
            Clock& operator=(const Clock&) = delete;
            Clock(Clock&&) = delete;
            Clock& operator=(Clock&&) = delete;

            void start() { m_startTime = std::chrono::high_resolution_clock::now(); }
            void stop();
            void resume();
            void update();
            static std::chrono::high_resolution_clock::time_point now() { return std::chrono::high_resolution_clock::now(); }

            [[nodiscard]] float getDeltaTime() const { return m_deltaTime.count(); }
            [[nodiscard]] float getDeltaTimeMS() const { return m_deltaTime.count() * 1000.F; }
            [[nodiscard]] float getFPS() const { return 1.F / m_deltaTime.count(); }

        private:

            TimePoint m_startTime;
            TimePoint m_stopTime;
            std::chrono::duration<float> m_deltaTime{0.F};

            bool m_isStopped{false};

    }; // class Clock

} // namespace utl
