#include "Aurorapch.h"
#include "ImGuiLayer.h"

#include "ImGuizmo.h"
#include "Core/Application.h"

#include "Theme.h"

#include <glm/glm.hpp>

#include <glfw/glfw3.h>

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

namespace Aurora {

	ImGuiLayer::ImGuiLayer()
		: Layer("ImGuiLayer")
	{
	}

	ImGuiLayer::~ImGuiLayer()
	{
	}

	void ImGuiLayer::OnAttach()
	{
		AR_PROFILE_FUNCTION();

		AR_CORE_INFO_TAG("ImGui", "ImGui version : {0}", IMGUI_VERSION);
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

		ImGui::StyleColorsDark();

		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 6.0f;
			style.PopupRounding = 3.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}

		LoadAndAddFonts();

		SetDarkThemeColor();

		Application& app = Application::GetApp();
		GLFWwindow* window = (GLFWwindow*)app.GetWindow().GetWindowPointer();

		ImGui_ImplGlfw_InitForOpenGL(window, true);
		ImGui_ImplOpenGL3_Init("#version 410");
	}

	void ImGuiLayer::OnDetach()
	{
		AR_PROFILE_FUNCTION();

		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	void ImGuiLayer::Begin()
	{
		AR_PROFILE_FUNCTION();

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGuizmo::BeginFrame();
	}

	void ImGuiLayer::End()
	{
		AR_PROFILE_FUNCTION();

		ImGuiIO& io = ImGui::GetIO();
		Application& app = Application::GetApp();
		io.DisplaySize = ImVec2((float)app.GetWindow().GetWidth(), (float)app.GetWindow().GetHeight());

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			GLFWwindow* backup_current_context = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(backup_current_context);
		}
	}

	void ImGuiLayer::SetDarkThemeColor()
	{
		ImGuiStyle& style = ImGui::GetStyle();
		auto& colors = ImGui::GetStyle().Colors;

		// Headers
		colors[ImGuiCol_Header]        = ImGui::ColorConvertU32ToFloat4(Theme::GroupHeader);
		colors[ImGuiCol_HeaderHovered] = ImGui::ColorConvertU32ToFloat4(Theme::GroupHeader);
		colors[ImGuiCol_HeaderActive]  = ImGui::ColorConvertU32ToFloat4(Theme::GroupHeader);

		// Buttons
		colors[ImGuiCol_Button]        = ImColor(56, 56, 56, 200);
		colors[ImGuiCol_ButtonHovered] = ImColor(70, 70, 70, 255);
		colors[ImGuiCol_ButtonActive]  = ImColor(56, 56, 56, 150);

		// Frame BG
		colors[ImGuiCol_FrameBg]        = ImGui::ColorConvertU32ToFloat4(Theme::PropertyField);
		colors[ImGuiCol_FrameBgHovered] = ImGui::ColorConvertU32ToFloat4(Theme::PropertyField);
		colors[ImGuiCol_FrameBgActive]  = ImGui::ColorConvertU32ToFloat4(Theme::PropertyField);

		// Scroll Bar
		colors[ImGuiCol_ScrollbarBg]          = ImVec4{ 0.02f, 0.02f, 0.02f, 0.53f };
		colors[ImGuiCol_ScrollbarGrab]        = ImVec4{ 0.31f, 0.31f, 0.31f, 1.0f };
		colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4{ 0.41f, 0.41f, 0.41f, 1.0f };
		colors[ImGuiCol_ScrollbarGrabActive]  = ImVec4{ 0.51f, 0.51f, 0.51f, 1.0f };

		// Check Mark
		colors[ImGuiCol_CheckMark] = ImColor(200, 200, 200, 255);

		// Slider Grab
		colors[ImGuiCol_SliderGrab]       = ImVec4(0.51f, 0.51f, 0.51f, 0.7f);
		colors[ImGuiCol_SliderGrabActive] = ImVec4(0.66f, 0.66f, 0.66f, 1.0f);

		// Text
		colors[ImGuiCol_Text] = ImGui::ColorConvertU32ToFloat4(Theme::Text);

		// Checkbox
		colors[ImGuiCol_CheckMark] = ImGui::ColorConvertU32ToFloat4(Theme::Text);

		// Separator
		colors[ImGuiCol_Separator]        = ImGui::ColorConvertU32ToFloat4(Theme::BackgroundDark);
		colors[ImGuiCol_SeparatorActive]  = ImGui::ColorConvertU32ToFloat4(Theme::Highlight);
		colors[ImGuiCol_SeparatorHovered] = ImColor(39, 185, 242, 150);

		// Resize Grip
		colors[ImGuiCol_ResizeGrip]        = ImVec4(0.91f, 0.91f, 0.91f, 0.25f);
		colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.81f, 0.81f, 0.81f, 0.67f);
		colors[ImGuiCol_ResizeGripActive]  = ImVec4(0.46f, 0.46f, 0.46f, 0.95f);

		// Window
		colors[ImGuiCol_WindowBg] = ImGui::ColorConvertU32ToFloat4(Theme::Titlebar);
		colors[ImGuiCol_ChildBg]  = ImGui::ColorConvertU32ToFloat4(Theme::Background);
		colors[ImGuiCol_PopupBg]  = ImGui::ColorConvertU32ToFloat4(Theme::BackgroundPopup);
		colors[ImGuiCol_Border]   = ImGui::ColorConvertU32ToFloat4(Theme::BackgroundDark);

		// Popup
		colors[ImGuiCol_PopupBg] = ImGui::ColorConvertU32ToFloat4(Theme::Background);

		// Tables
		colors[ImGuiCol_TableHeaderBg]    = ImGui::ColorConvertU32ToFloat4(Theme::GroupHeader);
		colors[ImGuiCol_TableBorderLight] = ImGui::ColorConvertU32ToFloat4(Theme::BackgroundDark);
		colors[ImGuiCol_TableRowBg]       = ImGui::ColorConvertU32ToFloat4(Theme::BackgroundMedium);
		colors[ImGuiCol_TableRowBgAlt]    = ImGui::ColorConvertU32ToFloat4(Theme::BackgroundDark);

		// MenuBar
		colors[ImGuiCol_MenuBarBg] = ImColor(0, 0, 0, 0);

		// Tabs
		colors[ImGuiCol_Tab]                = ImGui::ColorConvertU32ToFloat4(Theme::Titlebar);
		colors[ImGuiCol_TabHovered]         = ImColor(255, 255, 135, 30);
		colors[ImGuiCol_TabActive]          = ImGui::ColorConvertU32ToFloat4(Theme::Accent);
		colors[ImGuiCol_TabUnfocused]       = ImGui::ColorConvertU32ToFloat4(Theme::Titlebar);
		colors[ImGuiCol_TabUnfocusedActive] = ImGui::ColorConvertU32ToFloat4(Theme::AccentDimmed);

		// Title
		colors[ImGuiCol_TitleBg]          = ImGui::ColorConvertU32ToFloat4(Theme::Titlebar);
		colors[ImGuiCol_TitleBgActive]    = ImGui::ColorConvertU32ToFloat4(Theme::Titlebar);
		colors[ImGuiCol_TitleBgCollapsed] = ImColor(38.25f, 38.3775f, 38.505f, 255.0f);

		/// Style
		style.FrameRounding = 2.5f;
		style.FrameBorderSize = 1.0f;
		style.IndentSpacing = 11.0f;
	}

	void ImGuiLayer::LoadAndAddFonts()
	{
		AR_PROFILE_FUNCTION();

		// Loading default fonts
		FontSpecification robotoLarge = {};
		robotoLarge.FontName = "RobotoLarge";
		robotoLarge.FontFilePath = "Resources/EditorInternal/Fonts/Roboto/Roboto-Regular.ttf";
		robotoLarge.Size = 24.0f;
		m_FontsLibrary.LoadFont(robotoLarge);

		FontSpecification robotoBold = {};
		robotoBold.FontName = "RobotoBold";
		robotoBold.FontFilePath = "Resources/EditorInternal/Fonts/Roboto/Roboto-Bold.ttf";
		robotoBold.Size = 18.0f;
		m_FontsLibrary.LoadFont(robotoBold);

		FontSpecification robotoDefault = {};
		robotoDefault.FontName = "RobotoDefault";
		robotoDefault.FontFilePath = "Resources/EditorInternal/Fonts/Roboto/Roboto-SemiMedium.ttf";
		robotoDefault.Size = 15.0f;
		m_FontsLibrary.LoadFont(robotoDefault, true);

		static const ImWchar s_FontAwesomeGlyphRanges[] = { 0xf000, 0xf307, 0 };
		FontSpecification fontAwesome = {};
		fontAwesome.FontName = "FontAwesome";
		fontAwesome.FontFilePath = "Resources/EditorInternal/Fonts/FontAwesome/fontawesome-webfont.ttf";
		fontAwesome.Size = 16.0f;
		fontAwesome.GlyphRanges = s_FontAwesomeGlyphRanges;
		fontAwesome.MergeWithLast = true;
		m_FontsLibrary.LoadFont(fontAwesome);

		FontSpecification mochiyPop = {};
		mochiyPop.FontName = "MochiyPopOne";
		mochiyPop.FontFilePath = "Resources/EditorInternal/Fonts/MochiyPopOne/MochiyPopOne-Regular.ttf";
		mochiyPop.Size = 18.0f;
		m_FontsLibrary.LoadFont(mochiyPop);
	}

}