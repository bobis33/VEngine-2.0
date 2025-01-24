#include "Utils/Clock.hpp"

void utl::Clock::update()
{
    const auto newTime = std::chrono::high_resolution_clock::now();
    m_deltaTime = newTime - m_startTime;
    m_startTime = newTime;
}

void utl::Clock::stop()
{
    if (m_isStopped) {
        return;
    }

    m_stopTime = std::chrono::high_resolution_clock::now();
    m_isStopped = true;
}

void utl::Clock::resume()
{
    if (!m_isStopped) {
        return;
    }

    m_startTime += std::chrono::high_resolution_clock::now() - m_stopTime;
    m_isStopped = false;
}
