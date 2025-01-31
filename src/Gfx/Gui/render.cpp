#include <algorithm>
#include <fstream>
#include <numeric>
#include <unistd.h>

#include <imgui_impl_vulkan.h>
#include <imgui_impl_glfw.h>
#include <imgui.h>

#include "VEngine/Gfx/Gui.hpp"

static bool IsLegacyNativeDupe(const ImGuiKey key) { return key >= 0 && key < 512; };

void inputsSection(const ImGuiIO& io) {
    if (ImGui::BeginMenu("Input")) {
        ImGui::IsMousePosValid() ? ImGui::Text("Mouse pos: (%g, %g)", io.MousePos.x, io.MousePos.y) : ImGui::Text("Mouse pos: <INVALID>");
        ImGui::Text("Mouse delta: (%g, %g)", io.MouseDelta.x, io.MouseDelta.y);
        ImGui::Text("Mouse down:");
        for (int i = 0; i < static_cast<int>(std::size(io.MouseDown)); i++) {
            if (ImGui::IsMouseDown(i)) {
                ImGui::SameLine();
                ImGui::Text("b%d (%.02f secs)", i, io.MouseDownDuration[i]);
            }
        }
        ImGui::Text("Mouse wheel: %.1f", io.MouseWheel);
        ImGui::Text("Keys down:");
        for (auto key = static_cast<ImGuiKey>(0); key < ImGuiKey_NamedKey_END; key = static_cast<ImGuiKey>(key + 1)) {
            if (IsLegacyNativeDupe(key) || !ImGui::IsKeyDown(key)) { continue; }
            ImGui::SameLine();
            ImGui::Text((key < ImGuiKey_NamedKey_BEGIN) ? "\"%s\"" : "\"%s\" %d", ImGui::GetKeyName(key), key);
        }
        ImGui::EndMenu();
    }
}

void devicePropertiesSection(const VkPhysicalDeviceProperties& deviceProperties) {
    if (ImGui::BeginMenu("Device Properties")) {
        if (ImGui::BeginTable("DevicePropertiesTable", 2)) {
            ImGui::TableNextColumn(); ImGui::Text("Device Name: %s", deviceProperties.deviceName);
            ImGui::TableNextColumn(); ImGui::Text("API Version: %d.%d.%d", VK_VERSION_MAJOR(deviceProperties.apiVersion), VK_VERSION_MINOR(deviceProperties.apiVersion), VK_VERSION_PATCH(deviceProperties.apiVersion));
            ImGui::TableNextColumn(); ImGui::Text("Driver Version: %d.%d.%d", VK_VERSION_MAJOR(deviceProperties.driverVersion), VK_VERSION_MINOR(deviceProperties.driverVersion), VK_VERSION_PATCH(deviceProperties.driverVersion));
            ImGui::TableNextColumn();  ImGui::Text("Vendor ID: %d", deviceProperties.vendorID);
            ImGui::TableNextColumn(); ImGui::Text("Device ID: %d", deviceProperties.deviceID);
            ImGui::TableNextColumn(); ImGui::Text("Device Type: %d", deviceProperties.deviceType);
            ImGui::TableNextColumn(); ImGui::Text("Discrete Queue Priorities: %d", deviceProperties.limits.discreteQueuePriorities);
            ImGui::TableNextColumn(); ImGui::Text("Max Push Constants Size: %d", deviceProperties.limits.maxPushConstantsSize);
            ImGui::TableNextColumn(); ImGui::Text("Max Memory Allocation Count: %d", deviceProperties.limits.maxMemoryAllocationCount);
            ImGui::TableNextColumn(); ImGui::Text("Max Image Dimension 1D: %d", deviceProperties.limits.maxImageDimension1D);
            ImGui::TableNextColumn(); ImGui::Text("Max Image Dimension 2D: %d", deviceProperties.limits.maxImageDimension2D);
            ImGui::TableNextColumn(); ImGui::Text("Max Image Dimension 3D: %d", deviceProperties.limits.maxImageDimension3D);
            ImGui::TableNextColumn(); ImGui::Text("Max Image Dimension Cube: %d", deviceProperties.limits.maxImageDimensionCube);
            ImGui::TableNextColumn(); ImGui::Text("Max Image Array Layers: %d", deviceProperties.limits.maxImageArrayLayers);
            ImGui::TableNextColumn(); ImGui::Text("Max Texel Buffer Elements: %d", deviceProperties.limits.maxTexelBufferElements);
            ImGui::TableNextColumn(); ImGui::Text("Max Uniform Buffer Range: %d", deviceProperties.limits.maxUniformBufferRange);
            ImGui::TableNextColumn(); ImGui::Text("Max Storage Buffer Range: %d", deviceProperties.limits.maxStorageBufferRange);
            ImGui::EndTable();
        }
        ImGui::EndMenu();
    }
}

namespace FrameTimeGraphUtil {
    constexpr size_t MAX_FRAME_TIMES = 100;

    struct FrameTimeData {
        std::vector<float> frameTimes;
        size_t frameTimeIndex;

        FrameTimeData() : frameTimes(MAX_FRAME_TIMES, 0.0f), frameTimeIndex(0) {}
    };

    void UpdateFrameTimes(FrameTimeData& data, const float frameTime) {
        data.frameTimes[data.frameTimeIndex] = frameTime;
        data.frameTimeIndex = (data.frameTimeIndex + 1) % MAX_FRAME_TIMES;
    }

    std::vector<float> GetDisplayFrameTimes(const FrameTimeData& data) {
        std::vector<float> displayFrameTimes(MAX_FRAME_TIMES);
        for (size_t i = 0; i < MAX_FRAME_TIMES; ++i) {
            displayFrameTimes[i] = data.frameTimes[(data.frameTimeIndex + i) % MAX_FRAME_TIMES];
        }
        return displayFrameTimes;
    }

    std::vector<float> CalculateFPS(const std::vector<float>& frameTimes) {
        std::vector<float> fpsTimes(frameTimes.size());
        for (size_t i = 0; i < frameTimes.size(); ++i) {
            fpsTimes[i] = frameTimes[i] > 0 ? 1.0f / frameTimes[i] : 0.0f;
        }
        return fpsTimes;
    }

    void ExportDataToCSV(const std::vector<float>& frameTimes, const std::vector<float>& fpsTimes, const std::string& filename) {
        std::ofstream outFile(filename);
        for (size_t i = 0; i < frameTimes.size(); ++i) {
            outFile << frameTimes[i] * 1000.0f << ',' << fpsTimes[i] << '\n';
        }
        outFile.close();
    }

    float CalculateUpperBound(const std::vector<float>& data) {
        const float maxValue = *std::ranges::max_element(data);
        const float upperBound = maxValue * 1.2f;
        return upperBound < 0.0005f ? 0.0005f : upperBound;
    }
}

double get_memory_usage() {
    std::ifstream file("/proc/self/statm");
    long rss = 0;
    if (file >> rss) {
        return rss * sysconf(_SC_PAGE_SIZE) / (1024.0 * 1024.0); // Mémoire en Mo
    }
    return 0.0;
}

void DisplayMemoryStats() {
    std::ifstream file("/proc/meminfo");
    if (!file.is_open()) {
        ImGui::Text("Erreur lors de l'ouverture de /proc/meminfo");
        return;
    }
    static constexpr double KB_TO_MB = 1024.0;
    std::string line;
    double total_memory = 0.0;
    double available_memory = 0.0;
    double swap_total = 0.0;
    double swap_free = 0.0;
    std::array<char, 64> memoryUsage;
    std::array<char, 64> swapUsage;

    while (std::getline(file, line)) {
        if (line.starts_with("MemTotal:")) {
            total_memory = std::stod(line.substr(line.find(':') + 1)) / KB_TO_MB;
        } else if (line.starts_with("MemAvailable:")) {
            available_memory = std::stod(line.substr(line.find(':') + 1)) / KB_TO_MB;
        } else if (line.starts_with("SwapTotal:")) {
            swap_total = std::stod(line.substr(line.find(':') + 1)) / KB_TO_MB;
        } else if (line.starts_with("SwapFree:")) {
            swap_free = std::stod(line.substr(line.find(':') + 1)) / KB_TO_MB;
        }
    }
    snprintf(memoryUsage.data(), sizeof(memoryUsage), "Memory Usage: %.2f MB / %.2f MB", total_memory - available_memory, total_memory);
    ImGui::ProgressBar((total_memory - available_memory) / total_memory, ImVec2(280.0f, 0.0f), memoryUsage.data());
    ImGui::Text("Available: %.2f MB", available_memory);
    snprintf(swapUsage.data(), sizeof(swapUsage), "Swap Usage: %.2f MB / %.2f MB", swap_total - swap_free, swap_total);
    ImGui::ProgressBar((swap_total - swap_free) / swap_total, ImVec2(280.0f, 0.0f), swapUsage.data());
    ImGui::Text("Available: %.2f MB", swap_free);
}

void FrameTimeGraph(const float frameTime, const float frameRate) {
    using namespace FrameTimeGraphUtil;
    static FrameTimeData frameTimeData;
    UpdateFrameTimes(frameTimeData, frameTime);
    const auto displayFrameTimes = GetDisplayFrameTimes(frameTimeData);
    const auto fpsTimes = CalculateFPS(displayFrameTimes);
    const float upperBound = CalculateUpperBound(displayFrameTimes);
    ImGui::Text("FPS: %.1f", frameRate);
    ImGui::PlotHistogram(
        "##fps",
        fpsTimes.data(),
        MAX_FRAME_TIMES,
        0,
        nullptr,
        0.0f,
        10000.0f, // Max FPS
        ImVec2(290, 60)
    );
    ImGui::Text("Frame time: %.3f ms", 1000.0f / frameRate);
    ImGui::PlotHistogram(
            "##frame",
            displayFrameTimes.data(),
            MAX_FRAME_TIMES,
            0,
            nullptr,
            0.0f,
            upperBound,
            ImVec2(290, 60)
            );
    if (ImGui::Button("Export Data to CSV")) {
        ExportDataToCSV(displayFrameTimes, fpsTimes, "frametimes_and_fps.csv");
    }
    const float avgFrameTime = std::accumulate(displayFrameTimes.begin(), displayFrameTimes.end(), 0.0f) / MAX_FRAME_TIMES;
    const float minFrameTime = *std::ranges::min_element(displayFrameTimes);
    const float maxFrameTime = *std::ranges::max_element(displayFrameTimes);
    const float avgFPS = std::accumulate(fpsTimes.begin(), fpsTimes.end(), 0.0f) / MAX_FRAME_TIMES;
    const float minFPS = *std::ranges::min_element(fpsTimes);
    const float maxFPS = *std::ranges::max_element(fpsTimes);
    ImGui::Columns(2, nullptr, false);
    ImGui::Text("Avg Frame Time: %.3f ms", avgFrameTime * 1000.0f);
    ImGui::NextColumn();
    ImGui::Text("Avg FPS: %.1f", avgFPS);
    ImGui::NextColumn();
    ImGui::Text("Min Frame Time: %.3f ms", minFrameTime * 1000.0f);
    ImGui::NextColumn();
    ImGui::Text("Min FPS: %.1f", minFPS);
    ImGui::NextColumn();
    ImGui::Text("Max Frame Time: %.3f ms", maxFrameTime * 1000.0f);
    ImGui::NextColumn();
    ImGui::Text("Max FPS: %.1f", maxFPS);
    ImGui::Columns(1);
    DisplayMemoryStats();
}

void switchTheme() {
    static int selectedTheme = 0;
    static constexpr std::array themesNames = { "BlackWhite", "BlueGrey", "BlackRed" };
    if (ImGui::BeginMenu("Themes")) {
        for (int i = 0; i < themesNames.size(); ++i) {
            if (ImGui::MenuItem(themesNames[i], nullptr, selectedTheme == i)) {
                selectedTheme = i;
                switch (selectedTheme) {
                    case 0:
                        ven::Gui::applyTheme(ven::Gui::BlackWhite);
                    break;
                    case 1:
                        ven::Gui::applyTheme(ven::Gui::BlueGrey);
                    break;
                    case 2:
                        ven::Gui::applyTheme(ven::Gui::BlackRed);
                    break;
                    default:
                        ImGui::GetStyle() = ImGuiStyle();
                    break;
                }
            }
        }
        ImGui::Separator();
        if (ImGui::MenuItem("Reset Theme")) {
            ImGui::GetStyle() = ImGuiStyle();
            selectedTheme = -1;
        }
        ImGui::EndMenu();
    }
}

static void appInfo(const float frameRate) {
    ImGui::SetNextWindowPos({ImGui::GetIO().DisplaySize.x, 15.0F}, ImGuiCond_Always, {1.0f, 0.0f});
    ImGui::Begin("Application Info", nullptr,
                 ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
                 ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings |
                 ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav);
    ImGui::Text("%.1f FPS\t\n%.3f ms", frameRate, 1000.0f / frameRate);
    ImGui::End();
}

void ven::Gui::render(const VkCommandBuffer& commandBuffer) const {
    const ImGuiIO& io = ImGui::GetIO();
    const float frameRate = io.Framerate;
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    //ImGui::ShowMetricsWindow();
    //appInfo(frameRate);
    const float rightPanelWidth = 300.0f;
    ImGui::BeginMainMenuBar();
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - rightPanelWidth, 0), ImGuiCond_Always); // Alignement à droite
    ImGui::SetNextWindowSize(ImVec2(rightPanelWidth, io.DisplaySize.y), ImGuiCond_Always);    // Hauteur de la fenêtre
    ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImGui::GetStyle().Colors[ImGuiCol_FrameBg]); // Désactiver ActiveColor
    ImGui::PushStyleColor(ImGuiCol_TitleBg, ImGui::GetStyle().Colors[ImGuiCol_FrameBg]); // Désactiver TitleBg
    ImGui::PushStyleColor(ImGuiCol_TitleBgCollapsed, ImGui::GetStyle().Colors[ImGuiCol_FrameBg]); // Désactiver TitleBg
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImGui::GetStyle().Colors[ImGuiCol_FrameBg]); // Désactiver ActiveColor
    if (ImGui::Begin("##Right Panel", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse)) {
        FrameTimeGraph(io.DeltaTime, frameRate);
        ImGui::Separator();
        ImGui::Separator();
    }
    ImGui::End();
    ImGui::PopStyleColor();
    ImGui::PopStyleColor();
    ImGui::PopStyleColor();
    ImGui::PopStyleColor();
    inputsSection(io);
    switchTheme();
    devicePropertiesSection(m_deviceProperties);
    ImGui::EndMainMenuBar();
    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
}
