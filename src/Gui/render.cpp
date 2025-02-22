#include <algorithm>
#include <fstream>
#include <numeric>
#include <unistd.h>

#include <imgui_impl_vulkan.h>
#include <imgui_impl_glfw.h>
#include <imgui.h>

#include "VEngine/Gui/Gui.hpp"

static bool IsLegacyNativeDupe(const ImGuiKey key) { return key >= 0 && key < 512; }

void frameSection(const float frameRate, const ven::FrameStats& frameStats, float& graphMaxFps) {
    const auto displayFrameTimes = frameStats.getDisplayFrameTimes();
    const auto fpsTimes = ven::FrameStats::calculateFPS(displayFrameTimes);
    const float upperBound = ven::FrameStats::calculateUpperBound(displayFrameTimes);
    const float avgFrameTime = std::accumulate(displayFrameTimes.begin(), displayFrameTimes.end(), 0.0F) / ven::FrameStats::MAX_FRAME_TIMES;
    const float minFrameTime = *std::ranges::min_element(displayFrameTimes);
    const float maxFrameTime = *std::ranges::max_element(displayFrameTimes);
    const float avgFPS = std::accumulate(fpsTimes.begin(), fpsTimes.end(), 0.0F) / ven::FrameStats::MAX_FRAME_TIMES;
    const float minFPS = *std::ranges::min_element(fpsTimes);
    const float maxFPS = *std::ranges::max_element(fpsTimes);
    ImGui::Text("FPS: %.1f", frameRate);
    ImGui::PlotHistogram(
        "##fps",
        fpsTimes.data(),
        ven::FrameStats::MAX_FRAME_TIMES,
        0,
        nullptr,
        0.0F,
        graphMaxFps,
        ImVec2(290, 60)
    );
    ImGui::SliderFloat("Max scale", &graphMaxFps, 0.0F, 10000.0F);
    ImGui::Text("Frame time: %.3f ms", 1000.0F / frameRate);
    ImGui::PlotHistogram(
            "##frame",
            displayFrameTimes.data(),
            ven::FrameStats::MAX_FRAME_TIMES,
            0,
            nullptr,
            0.0F,
            upperBound,
            ImVec2(290, 60)
     );
    if (ImGui::Button("Export Data to CSV")) {
        frameStats.exportDataToCSV(displayFrameTimes, fpsTimes, "frametimes_and_fps.csv");
    }
    ImGui::Columns(2, nullptr, false);
    ImGui::Text("Avg Frame Time: %.3f ms", avgFrameTime * 1000.0F);
    ImGui::NextColumn();
    ImGui::Text("Avg FPS: %.1f", avgFPS);
    ImGui::NextColumn();
    ImGui::Text("Min Frame Time: %.3f ms", minFrameTime * 1000.0F);
    ImGui::NextColumn();
    ImGui::Text("Min FPS: %.1f", minFPS);
    ImGui::NextColumn();
    ImGui::Text("Max Frame Time: %.3f ms", maxFrameTime * 1000.0F);
    ImGui::NextColumn();
    ImGui::Text("Max FPS: %.1f", maxFPS);
    ImGui::Columns(1);
}

void memorySection(const ven::MemoryMonitor& memoryMonitor) {
    const double totalMemory = memoryMonitor.getTotalMemory();
    const double usedMemory = memoryMonitor.getMemoryUsage();
    const double availableMemory = memoryMonitor.getAvailableMemory();
    const double totalSwap = memoryMonitor.getTotalSwap();
    const double usedSwap = memoryMonitor.getSwapUsage();
    const double freeSwap = memoryMonitor.getFreeSwap();
    std::array<char, 64> memoryUsage;
    std::array<char, 64> swapUsage;
    snprintf(memoryUsage.data(), sizeof(memoryUsage), "Memory Usage: %.2f MB / %.2f MB", usedMemory, totalMemory);
    ImGui::ProgressBar(usedMemory / totalMemory, ImVec2(280.0F, 0.0F), memoryUsage.data());
    ImGui::Text("Available: %.2f MB", availableMemory);
    snprintf(swapUsage.data(), sizeof(swapUsage), "Swap Usage: %.2f MB / %.2f MB", usedSwap, totalSwap);
    ImGui::ProgressBar(usedSwap / totalSwap, ImVec2(280.0F, 0.0F), swapUsage.data());
    ImGui::Text("Available: %.2f MB", freeSwap);
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

void themeSection() {
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

void ven::Gui::render(const VkCommandBuffer& commandBuffer) {
    const ImGuiIO& io = ImGui::GetIO();
    const float frameRate = io.Framerate;
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    //ImGui::ShowMetricsWindow();
    //appInfo(frameRate);
    constexpr float rightPanelWidth = 300.0F;
    ImGui::BeginMainMenuBar();
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - rightPanelWidth, 0), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(rightPanelWidth, io.DisplaySize.y), ImGuiCond_Always);
    ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImGui::GetStyle().Colors[ImGuiCol_FrameBg]);
    ImGui::PushStyleColor(ImGuiCol_TitleBg, ImGui::GetStyle().Colors[ImGuiCol_FrameBg]);
    ImGui::PushStyleColor(ImGuiCol_TitleBgCollapsed, ImGui::GetStyle().Colors[ImGuiCol_FrameBg]);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImGui::GetStyle().Colors[ImGuiCol_FrameBg]);
    if (ImGui::Begin("##Right Panel", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse)) {
        m_frameStats.updateFrameTimes(io.DeltaTime);
        frameSection(frameRate, m_frameStats, m_graphMaxFps);
        m_memoryMonitor.update();
        memorySection(m_memoryMonitor);
        ImGui::Separator();
        ImGui::Separator();
    }
    ImGui::End();
    ImGui::PopStyleColor();
    ImGui::PopStyleColor();
    ImGui::PopStyleColor();
    ImGui::PopStyleColor();
    inputsSection(io);
    themeSection();
    devicePropertiesSection(m_deviceProperties);
    ImGui::EndMainMenuBar();
    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
}
