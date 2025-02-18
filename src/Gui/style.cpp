#include <imgui.h>

#include "VEngine/Gui/Gui.hpp"

static constexpr std::array<ImVec4, ImGuiCol_COUNT> themeColors = {
    ImVec4(0.00F, 0.00F, 0.00F, 1.00F), // ImGuiCol_Text
    ImVec4(0.00F, 0.00F, 0.00F, 1.00F), // ImGuiCol_TextDisabled
    ImVec4(0.00F, 0.00F, 0.00F, 1.00F), // ImGuiCol_WindowBg
    ImVec4(0.00F, 0.00F, 0.00F, 1.00F), // ImGuiCol_ChildBg
    ImVec4(0.00F, 0.00F, 0.00F, 1.00F), // ImGuiCol_PopupBg
    ImVec4(0.00F, 0.00F, 0.00F, 1.00F), // ImGuiCol_Border
    ImVec4(0.00F, 0.00F, 0.00F, 1.00F), // ImGuiCol_BorderShadow
    ImVec4(0.00F, 0.00F, 0.00F, 1.00F), // ImGuiCol_FrameBg
    ImVec4(0.00F, 0.00F, 0.00F, 1.00F), // ImGuiCol_FrameBgHovered
    ImVec4(0.00F, 0.00F, 0.00F, 1.00F), // ImGuiCol_FrameBgActive
    ImVec4(0.00F, 0.00F, 0.00F, 1.00F), // ImGuiCol_TitleBg
    ImVec4(0.00F, 0.00F, 0.00F, 1.00F), // ImGuiCol_TitleBgActive
    ImVec4(0.00F, 0.00F, 0.00F, 1.00F), // ImGuiCol_TitleBgCollapsed
    ImVec4(0.00F, 0.00F, 0.00F, 1.00F), // ImGuiCol_MenuBarBg
    ImVec4(0.00F, 0.00F, 0.00F, 1.00F), // ImGuiCol_ScrollbarBg
    ImVec4(0.00F, 0.00F, 0.00F, 1.00F), // ImGuiCol_ScrollbarGrab
    ImVec4(0.00F, 0.00F, 0.00F, 1.00F), // ImGuiCol_ScrollbarGrabHovered
    ImVec4(0.00F, 0.00F, 0.00F, 1.00F), // ImGuiCol_ScrollbarGrabActive
    ImVec4(0.00F, 0.00F, 0.00F, 1.00F), // ImGuiCol_CheckMark
    ImVec4(0.00F, 0.00F, 0.00F, 1.00F), // ImGuiCol_SliderGrab
    ImVec4(0.00F, 0.00F, 0.00F, 1.00F), // ImGuiCol_SliderGrabActive
    ImVec4(0.00F, 0.00F, 0.00F, 1.00F), // ImGuiCol_Button
    ImVec4(0.00F, 0.00F, 0.00F, 1.00F), // ImGuiCol_ButtonHovered
    ImVec4(0.00F, 0.00F, 0.00F, 1.00F), // ImGuiCol_ButtonActive
    ImVec4(0.00F, 0.00F, 0.00F, 1.00F), // ImGuiCol_Header
    ImVec4(0.00F, 0.00F, 0.00F, 1.00F), // ImGuiCol_HeaderHovered
    ImVec4(0.00F, 0.00F, 0.00F, 1.00F), // ImGuiCol_HeaderActive
    ImVec4(0.00F, 0.00F, 0.00F, 1.00F), // ImGuiCol_Separator
    ImVec4(0.00F, 0.00F, 0.00F, 1.00F), // ImGuiCol_SeparatorHovered
    ImVec4(0.00F, 0.00F, 0.00F, 1.00F), // ImGuiCol_SeparatorActive
    ImVec4(0.00F, 0.00F, 0.00F, 1.00F), // ImGuiCol_ResizeGrip
    ImVec4(0.00F, 0.00F, 0.00F, 1.00F), // ImGuiCol_ResizeGripHovered
    ImVec4(0.00F, 0.00F, 0.00F, 1.00F), // ImGuiCol_ResizeGripActive
    ImVec4(0.00F, 0.00F, 0.00F, 1.00F), // ImGuiCol_TabHovered
    ImVec4(0.00F, 0.00F, 0.00F, 1.00F), // ImGuiCol_Tab
    ImVec4(0.00F, 0.00F, 0.00F, 1.00F), // ImGuiCol_TabSelected
    ImVec4(0.00F, 0.00F, 0.00F, 1.00F), // ImGuiCol_TabSelectedOverline
    ImVec4(0.00F, 0.00F, 0.00F, 1.00F), // ImGuiCol_TabDimmed
    ImVec4(0.00F, 0.00F, 0.00F, 1.00F), // ImGuiCol_TabDimmedSelected
    ImVec4(0.00F, 0.00F, 0.00F, 1.00F), // ImGuiCol_TabDimmedSelectedOverline
    ImVec4(0.00F, 0.00F, 0.00F, 1.00F), // ImGuiCol_PlotLines
    ImVec4(0.00F, 0.00F, 0.00F, 1.00F), // ImGuiCol_PlotLinesHovered
    ImVec4(0.00F, 0.00F, 0.00F, 1.00F), // ImGuiCol_PlotHistogram
    ImVec4(0.00F, 0.00F, 0.00F, 1.00F), // ImGuiCol_PlotHistogramHovered
    ImVec4(0.00F, 0.00F, 0.00F, 1.00F), // ImGuiCol_TableHeaderBg
    ImVec4(0.00F, 0.00F, 0.00F, 1.00F), // ImGuiCol_TableBorderStrong
    ImVec4(0.00F, 0.00F, 0.00F, 1.00F), // ImGuiCol_TableBorderLight
    ImVec4(0.00F, 0.00F, 0.00F, 1.00F), // ImGuiCol_TableRowBg
    ImVec4(0.00F, 0.00F, 0.00F, 1.00F), // ImGuiCol_TableRowBgAlt
    ImVec4(0.00F, 0.00F, 0.00F, 1.00F), // ImGuiCol_TextLink
    ImVec4(0.00F, 0.00F, 0.00F, 1.00F), // ImGuiCol_TextSelectedBg
    ImVec4(0.00F, 0.00F, 0.00F, 1.00F), // ImGuiCol_DragDropTarget
    ImVec4(0.00F, 0.00F, 0.00F, 1.00F), // ImGuiCol_NavCursor
    ImVec4(0.00F, 0.00F, 0.00F, 1.00F), // ImGuiCol_NavWindowingHighlight
    ImVec4(0.00F, 0.00F, 0.00F, 1.00F), // ImGuiCol_NavWindowingDimBg
    ImVec4(0.00F, 0.00F, 0.00F, 1.00F), // ImGuiCol_ModalWindowDimBg
};

void ven::Gui::blueGreyTheme() {
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

void ven::Gui::blackRedTheme() {
    ImGuiStyle& style = ImGui::GetStyle();
    style.Colors[ImGuiCol_Text]                  = ImVec4(0.86f, 0.93f, 0.89f, 0.78f);
    style.Colors[ImGuiCol_TextDisabled]          = ImVec4(0.86f, 0.93f, 0.89f, 0.28f);
    style.Colors[ImGuiCol_WindowBg]              = ImVec4(0.13f, 0.14f, 0.17f, 1.00f);
    style.Colors[ImGuiCol_Border]                = ImVec4(0.31f, 0.31f, 1.00f, 0.00f);
    style.Colors[ImGuiCol_BorderShadow]          = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    style.Colors[ImGuiCol_FrameBg]               = ImVec4(0.20f, 0.22f, 0.27f, 1.00f);
    style.Colors[ImGuiCol_FrameBgHovered]        = ImVec4(0.92f, 0.18f, 0.29f, 0.78f);
    style.Colors[ImGuiCol_FrameBgActive]         = ImVec4(0.92f, 0.18f, 0.29f, 1.00f);
    style.Colors[ImGuiCol_TitleBg]               = ImVec4(0.20f, 0.22f, 0.27f, 1.00f);
    style.Colors[ImGuiCol_TitleBgCollapsed]      = ImVec4(0.20f, 0.22f, 0.27f, 0.75f);
    style.Colors[ImGuiCol_TitleBgActive]         = ImVec4(0.92f, 0.18f, 0.29f, 1.00f);
    style.Colors[ImGuiCol_MenuBarBg]             = ImVec4(0.20f, 0.22f, 0.27f, 1.00f);
    style.Colors[ImGuiCol_ScrollbarBg]           = ImVec4(0.20f, 0.22f, 0.27f, 1.00f);
    style.Colors[ImGuiCol_ScrollbarGrab]         = ImVec4(0.09f, 0.15f, 0.16f, 1.00f);
    style.Colors[ImGuiCol_ScrollbarGrabHovered]  = ImVec4(0.92f, 0.18f, 0.29f, 0.78f);
    style.Colors[ImGuiCol_ScrollbarGrabActive]   = ImVec4(0.92f, 0.18f, 0.29f, 1.00f);
    style.Colors[ImGuiCol_CheckMark]             = ImVec4(0.71f, 0.22f, 0.27f, 1.00f);
    style.Colors[ImGuiCol_SliderGrab]            = ImVec4(0.47f, 0.77f, 0.83f, 0.14f);
    style.Colors[ImGuiCol_SliderGrabActive]      = ImVec4(0.92f, 0.18f, 0.29f, 1.00f);
    style.Colors[ImGuiCol_Button]                = ImVec4(0.47f, 0.77f, 0.83f, 0.14f);
    style.Colors[ImGuiCol_ButtonHovered]         = ImVec4(0.92f, 0.18f, 0.29f, 0.86f);
    style.Colors[ImGuiCol_ButtonActive]          = ImVec4(0.92f, 0.18f, 0.29f, 1.00f);
    style.Colors[ImGuiCol_Header]                = ImVec4(0.92f, 0.18f, 0.29f, 0.76f);
    style.Colors[ImGuiCol_HeaderHovered]         = ImVec4(0.92f, 0.18f, 0.29f, 0.86f);
    style.Colors[ImGuiCol_HeaderActive]          = ImVec4(0.92f, 0.18f, 0.29f, 1.00f);
    style.Colors[ImGuiCol_Separator]             = ImVec4(0.14f, 0.16f, 0.19f, 1.00f);
    style.Colors[ImGuiCol_SeparatorHovered]      = ImVec4(0.92f, 0.18f, 0.29f, 0.78f);
    style.Colors[ImGuiCol_SeparatorActive]       = ImVec4(0.92f, 0.18f, 0.29f, 1.00f);
    style.Colors[ImGuiCol_ResizeGrip]            = ImVec4(0.47f, 0.77f, 0.83f, 0.04f);
    style.Colors[ImGuiCol_ResizeGripHovered]     = ImVec4(0.92f, 0.18f, 0.29f, 0.78f);
    style.Colors[ImGuiCol_ResizeGripActive]      = ImVec4(0.92f, 0.18f, 0.29f, 1.00f);
    style.Colors[ImGuiCol_PlotLines]             = ImVec4(0.86f, 0.93f, 0.89f, 0.63f);
    style.Colors[ImGuiCol_PlotLinesHovered]      = ImVec4(0.92f, 0.18f, 0.29f, 1.00f);
    style.Colors[ImGuiCol_PlotHistogram]         = ImVec4(0.86f, 0.93f, 0.89f, 0.63f);
    style.Colors[ImGuiCol_PlotHistogramHovered]  = ImVec4(0.92f, 0.18f, 0.29f, 1.00f);
    style.Colors[ImGuiCol_TextSelectedBg]        = ImVec4(0.92f, 0.18f, 0.29f, 0.43f);
    style.Colors[ImGuiCol_PopupBg]               = ImVec4(0.20f, 0.22f, 0.27f, 0.9f);
}

void ven::Gui::blackWhiteTheme() {
    ImGuiStyle& style = ImGui::GetStyle();

    style.WindowRounding = 0.0f;
    style.FrameRounding = 0.0f;
    style.GrabRounding = 0.0f;
    style.ScrollbarRounding = 0.0f;
    style.TabRounding = 0.0f;

    style.Colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.24f, 0.24f, 0.24f, 1.00f);
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    style.Colors[ImGuiCol_ChildBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    style.Colors[ImGuiCol_PopupBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    style.Colors[ImGuiCol_Border] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
    style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    style.Colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.24f, 0.24f, 0.24f, 1.00f);
    style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.24f, 0.24f, 0.24f, 1.00f);
    style.Colors[ImGuiCol_TitleBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.24f, 0.24f, 0.24f, 1.00f);
    style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.24f, 0.24f, 0.24f, 1.00f);
    style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.24f, 0.24f, 0.24f, 1.00f);
    style.Colors[ImGuiCol_CheckMark] = ImVec4(0.86f, 0.93f, 0.89f, 1.00f);
    style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.24f, 0.24f, 0.24f, 1.00f);
    style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.24f, 0.24f, 0.24f, 1.00f);
    style.Colors[ImGuiCol_Button] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.24f, 0.24f, 0.24f, 1.00f);
    style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.24f, 0.24f, 0.24f, 1.00f);
    style.Colors[ImGuiCol_Header] = ImVec4(0.24f, 0.24f, 0.24f, 1.00f);
    style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.24f, 0.24f, 0.24f, 1.00f);
    style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.24f, 0.24f, 0.24f, 1.00f);
    style.Colors[ImGuiCol_Separator] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
    style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
    style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
    style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.24f, 0.24f, 0.24f, 1.00f);
    style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.24f, 0.24f, 0.24f, 1.00f);
    style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.24f, 0.24f, 0.24f, 1.00f);
    style.Colors[ImGuiCol_Tab] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    style.Colors[ImGuiCol_TabHovered] = ImVec4(0.24f, 0.24f, 0.24f, 1.00f);
    style.Colors[ImGuiCol_TabActive] = ImVec4(0.24f, 0.24f, 0.24f, 1.00f);
    style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.24f, 0.24f, 0.24f, 1.00f);
    style.Colors[ImGuiCol_PlotLines] = ImVec4(0.86f, 0.93f, 0.89f, 1.00f);
    style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.86f, 0.93f, 0.89f, 1.00f);
    style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.86f, 0.93f, 0.89f, 1.00f);
    style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.86f, 0.93f, 0.89f, 1.00f);
    style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.24f, 0.24f, 0.24f, 1.00f);
    style.Colors[ImGuiCol_DragDropTarget] = ImVec4(0.86f, 0.93f, 0.89f, 1.00f);
    style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.86f, 0.93f, 0.89f, 1.00f);
    style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(0.86f, 0.93f, 0.89f, 1.00f);
    style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
}