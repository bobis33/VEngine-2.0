#include "VEngine/Gfx/Gui.hpp"

static bool IsLegacyNativeDupe(const ImGuiKey key) { return key >= 0 && key < 512; };

static void initStyle() {
    ImGuiStyle& style = ImGui::GetStyle();
    style.Alpha = 1.0;
    style.WindowRounding = 3;
    style.GrabRounding = 1;
    style.GrabMinSize = 20;
    style.FrameRounding = 3;
    style.Colors[ImGuiCol_Text] = ImVec4(0.00F, 1.00F, 1.00F, 1.00F);
    style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.00F, 0.40F, 0.41F, 1.00F);
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.1F, 0.1F, 0.1F, 0.70F);
    style.Colors[ImGuiCol_Border] = ImVec4(0.00F, 1.00F, 1.00F, 0.35F);
    style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.00F, 0.00F, 0.00F, 0.00F);
    style.Colors[ImGuiCol_FrameBg] = ImVec4(0.44F, 0.80F, 0.80F, 0.18F);
    style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.44F, 0.80F, 0.80F, 0.27F);
    style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.44F, 0.81F, 0.86F, 0.66F);
    style.Colors[ImGuiCol_TitleBg] = ImVec4(0.14F, 0.18F, 0.21F, 0.73F);
    style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00F, 0.00F, 0.00F, 0.54F);
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.00F, 1.00F, 1.00F, 0.27F);
    style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.00F, 0.00F, 0.00F, 0.20F);
    style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.22F, 0.29F, 0.30F, 0.71F);
    style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.00F, 1.00F, 1.00F, 0.44F);
    style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.00F, 1.00F, 1.00F, 0.74F);
    style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.00F, 1.00F, 1.00F, 1.00F);
    style.Colors[ImGuiCol_CheckMark] = ImVec4(0.00F, 1.00F, 1.00F, 0.68F);
    style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.00F, 1.00F, 1.00F, 0.36F);
    style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.00F, 1.00F, 1.00F, 0.76F);
    style.Colors[ImGuiCol_Button] = ImVec4(0.00F, 0.65F, 0.65F, 0.46F);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.01F, 1.00F, 1.00F, 0.43F);
    style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.00F, 1.00F, 1.00F, 0.62F);
    style.Colors[ImGuiCol_Header] = ImVec4(0.00F, 1.00F, 1.00F, 0.33F);
    style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.00F, 1.00F, 1.00F, 0.42F);
    style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.00F, 1.00F, 1.00F, 0.54F);
    style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.00F, 1.00F, 1.00F, 0.54F);
    style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.00F, 1.00F, 1.00F, 0.74F);
    style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.00F, 1.00F, 1.00F, 1.00F);
    style.Colors[ImGuiCol_PlotLines] = ImVec4(0.00F, 1.00F, 1.00F, 1.00F);
    style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.00F, 1.00F, 1.00F, 1.00F);
    style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.00F, 1.00F, 1.00F, 1.00F);
    style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.00F, 1.00F, 1.00F, 1.00F);
    style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.00F, 1.00F, 1.00F, 0.22F);
}

void createDescriptorPool(const VkDevice& device, VkDescriptorPool& pool) {
    static constexpr uint16_t DESCRIPTOR_COUNT = 1000;
    static constexpr std::array<VkDescriptorPoolSize, 11> poolSizes = {{
        { .type=VK_DESCRIPTOR_TYPE_SAMPLER, .descriptorCount=DESCRIPTOR_COUNT },
        { .type=VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .descriptorCount=DESCRIPTOR_COUNT },
        { .type=VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, .descriptorCount=DESCRIPTOR_COUNT},
        { .type=VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, .descriptorCount=DESCRIPTOR_COUNT },
        { .type=VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, .descriptorCount=DESCRIPTOR_COUNT },
        { .type=VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, .descriptorCount=DESCRIPTOR_COUNT },
        { .type=VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .descriptorCount=DESCRIPTOR_COUNT },
        { .type=VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, .descriptorCount=DESCRIPTOR_COUNT },
        { .type=VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, .descriptorCount=DESCRIPTOR_COUNT },
        { .type=VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, .descriptorCount=DESCRIPTOR_COUNT },
        { .type=VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, .descriptorCount=DESCRIPTOR_COUNT }}};
    static constexpr VkDescriptorPoolCreateInfo poolInfo {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
        .maxSets = DESCRIPTOR_COUNT,
        .poolSizeCount = poolSizes.size(),
        .pPoolSizes = poolSizes.data()
    };
    if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &pool) != VK_SUCCESS) {
        utl::THROW_ERROR("Failed to create ImGui descriptor pool");
    }
}

void inputsSection(const ImGuiIO& io) {
    if (ImGui::CollapsingHeader("Input")) {
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
    }
}

void devicePropertiesSection(const VkPhysicalDeviceProperties& deviceProperties) {
    if (ImGui::CollapsingHeader("Device Properties")) {
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
    }
}

void ven::Gui::render(const VkCommandBuffer& commandBuffer) const {
    const ImGuiIO& io = ImGui::GetIO();
    const float frameRate = io.Framerate;
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    //ImGui::ShowMetricsWindow();
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
    ImGui::Begin("Application Info", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav);
    ImGui::Text("%.1f FPS\t\n%.3f ms", frameRate, 1000.0f / frameRate);
    ImGui::End();
    ImGui::SetNextWindowPos(ImVec2(0, 43), ImGuiCond_Always);
    ImGui::Begin("Infos");
    ImGui::Text("%d vertices", io.MetricsRenderVertices);
    ImGui::Text("%d indices", io.MetricsRenderIndices);
    ImGui::Text("%d triangles", io.MetricsRenderIndices / 3);
    inputsSection(io);
    devicePropertiesSection(m_deviceProperties);
    ImGui::End();
    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
}

ven::Gui::Gui(const Device& device, GLFWwindow* window): m_device(device.getVkDevice())
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::GetIO().IniFilename = nullptr;
    createDescriptorPool(m_device, m_pool);

    ImGui_ImplVulkan_InitInfo init_info{};
    init_info.Instance = device.getVkInstance();
    init_info.PhysicalDevice = device.getPhysicalDevice();
    init_info.Device = device.getVkDevice();
    init_info.QueueFamily = device.findQueueFamilies(device.getPhysicalDevice()).graphicsFamily.value();
    init_info.Queue = device.getGraphicsQueue();
    init_info.PipelineCache = nullptr;
    init_info.DescriptorPool = m_pool;
    init_info.MinImageCount = 3;
    init_info.ImageCount = 3;
    init_info.MSAASamples = device.getMsaaSamples();
    init_info.Subpass = 0;
    ImGui_ImplVulkan_Init(&init_info);
    ImGui_ImplGlfw_InitForVulkan(window, true);
    vkGetPhysicalDeviceProperties(device.getPhysicalDevice(), &m_deviceProperties);
    initStyle();
}
