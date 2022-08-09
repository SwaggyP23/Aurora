#include "Aurorapch.h"
#include "ImGuiLayer.h"

#include "Core/Application.h"

#include <glm/glm.hpp>

#include <glfw/glfw3.h>

#include <ImGui/imgui.h>
#include <ImGui/imgui_impl_glfw.h>
#include <ImGui/imgui_impl_opengl3.h>
#include <ImGuizmo/ImGuizmo.h>

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

		AR_CORE_INFO("[ImGui]: ImGui version: {0}", IMGUI_VERSION);
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
			style.PopupRounding = 6.0f;
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

	void ImGuiLayer::OnEvent(Event& e)
	{
		if (m_BlockEvents)
		{
			ImGuiIO& io = ImGui::GetIO();
			e.Handled |= e.IsInCategory(EventCategoryMouse) & io.WantCaptureMouse;
			e.Handled |= e.IsInCategory(EventCategoryKeyboard) & io.WantCaptureKeyboard;
		}
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
		AR_PROFILE_FUNCTION();

		auto& colors = ImGui::GetStyle().Colors;
		colors[ImGuiCol_WindowBg] = ImVec4{ 0.1f, 0.105f, 0.11f, 1.0f };

		// Headers
		colors[ImGuiCol_Header] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
		colors[ImGuiCol_HeaderHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
		colors[ImGuiCol_HeaderActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

		// Buttons
		colors[ImGuiCol_Button] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
		colors[ImGuiCol_ButtonHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
		colors[ImGuiCol_ButtonActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

		// Frame BG
		colors[ImGuiCol_FrameBg] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
		colors[ImGuiCol_FrameBgHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
		colors[ImGuiCol_FrameBgActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

		// Scroll Bar
		//colors[ImGuiCol_ScrollbarBg] = ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f };
		colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4{ 79.0f / 255.0f, 162.0f / 255.0f, 175.0f / 255.0f, 1.0f };
		colors[ImGuiCol_ScrollbarGrabActive] = ImVec4{ 252.0f / 255.0f, 180.0f / 255.0f, 36.0f / 255.0f, 1.0f };

		// Check Mark
		colors[ImGuiCol_CheckMark] = ImVec4{ 252.0f / 255.0f, 180.0f / 255.0f, 36.0f / 255.0f, 1.0f };

		// Slider Grab
		colors[ImGuiCol_SliderGrab] = ImVec4{ 252.0f / 255.0f, 180.0f / 255.0f, 36.0f / 255.0f, 1.0f };
		colors[ImGuiCol_SliderGrabActive] = ImVec4{ 250.0f / 255.0f, 83.0f / 255.0f , 147.0f / 255.0f, 1.0f };

		// Resize Grip
		colors[ImGuiCol_ResizeGrip] = ImVec4{ 250.0f / 255.0f, 83.0f / 255.0f , 147.0f / 255.0f, 1.0f };
		colors[ImGuiCol_ResizeGripHovered] = ImVec4{ 79.0f / 255.0f, 162.0f / 255.0f, 175.0f / 255.0f, 1.0f };
		colors[ImGuiCol_ResizeGripActive] = ImVec4{ 252.0f / 255.0f, 180.0f / 255.0f, 36.0f/ 255.0f, 1.0f };

		// Window
		colors[ImGuiCol_WindowBg] = ImVec4{ 0.06f, 0.06f, 0.06f, 1.0f };

		// Tabs
		colors[ImGuiCol_Tab] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
		colors[ImGuiCol_TabHovered] = ImVec4{ 0.38f, 0.3805f, 0.381f, 1.0f };
		colors[ImGuiCol_TabActive] = ImVec4{ 0.28f, 0.2805f, 0.281f, 1.0f };
		colors[ImGuiCol_TabUnfocused] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
		colors[ImGuiCol_TabUnfocusedActive] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };

		// Title
		colors[ImGuiCol_TitleBg] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
		colors[ImGuiCol_TitleBgActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
	}

	void ImGuiLayer::LoadAndAddFonts()
	{
		AR_PROFILE_FUNCTION();

		// Loading default fonts
		m_Fonts.LoadFont("OpenSans", "resources/fonts/OpenSans");
		m_Fonts.LoadFont("Teko", "resources/fonts/Teko");
		m_Fonts.LoadFont("BebasNeue", "resources/fonts/BebasNeue");
		m_Fonts.LoadFont("Edu NSW ACT", "resources/fonts/Edu NSW ACT Foundation");
		m_Fonts.LoadFont("MochiyPopOne", "resources/fonts/MochiyPopOne");
		m_Fonts.LoadFont("Chewy", "resources/fonts/Chewy");

		m_Fonts.AddFont("OpenSans", FontIdentifier::Bold);
		m_Fonts.AddFont("OpenSans", FontIdentifier::Italic);
		m_Fonts.AddFont("OpenSans", FontIdentifier::Regular);
		m_Fonts.AddFont("OpenSans", FontIdentifier::Medium);
		m_Fonts.AddFont("OpenSans", FontIdentifier::Light);

		m_Fonts.AddFont("Teko", FontIdentifier::Bold, 22.0f);
		m_Fonts.AddFont("Teko", FontIdentifier::Regular, 25.0f);
		m_Fonts.AddFont("Teko", FontIdentifier::Medium, 25.0f);
		m_Fonts.AddFont("Teko", FontIdentifier::Light, 25.0f);

		m_Fonts.AddFont("Edu NSW ACT", FontIdentifier::Bold, 19.5f);
		m_Fonts.AddFont("Edu NSW ACT", FontIdentifier::Regular, 19.5f);
		m_Fonts.AddFont("Edu NSW ACT", FontIdentifier::Medium, 19.5f);

		m_Fonts.AddFont("MochiyPopOne", FontIdentifier::Regular, 18.0f);

		m_Fonts.AddFont("Chewy", FontIdentifier::Regular, 20.0f);

		m_Fonts.AddFont("BebasNeue", FontIdentifier::Regular, 22.0f);

		// More fonts are to be added

		m_Fonts.SetDefaultFont("OpenSans", FontIdentifier::Medium, 30.0f);
	}

}