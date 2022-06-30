#include "OGLpch.h"
#include "ImGuiLayer.h"

#include <ImGui/imgui.h>
#include <ImGui/imgui_impl_glfw.h>
#include <ImGui/imgui_impl_opengl3.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Application.h"

ImGuiLayer::ImGuiLayer()
	: Layer("ImGui")
{
}

ImGuiLayer::~ImGuiLayer()
{
}

void ImGuiLayer::onAttach()
{
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
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}
	
	Application& app = Application::getApp();
	GLFWwindow* window = (GLFWwindow*)app.getWindow().getWindowPointer();

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 410");
}

void ImGuiLayer::onDetach()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

void ImGuiLayer::begin()
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

void ImGuiLayer::end()
{
	ImGuiIO& io = ImGui::GetIO();
	Application& app = Application::getApp();
	io.DisplaySize = ImVec2((float)app.getWindow().getWidth(), (float)app.getWindow().getHeight());

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

void ImGuiLayer::onImGuiRender()
{
	//static bool show = true;
	//ImGui::ShowDemoWindow(&show);

	Application& app = Application::getApp();

	ImGui::Begin("Editing Panel");
	if (ImGui::CollapsingHeader("Cube")) {
		//ImGui::Begin("Editing");
		ImGui::ColorEdit3("Uniform Color", (float*)&m_UniColor);
		ImGui::SliderFloat3("Translation", (float*)&m_Transalations, 0.0f, 5.0f);
		ImGui::SliderFloat("Rotation", (float*)&m_Rotation, 0.0f, 2.0f);
		ImGui::SliderFloat3("Scale", (float*)&m_Scale, 0.0f, 3.0f);
		ImGui::SliderFloat("Blend", &m_Blend, 0.0f, 1.0f);
		//ImGui::End();
	}

	ImGui::Separator();

	if (ImGui::CollapsingHeader("Light Source")) {
		//ImGui::Begin("Editing");
		ImGui::ColorEdit3("Light Color", (float*)&m_LightColor);
		ImGui::SliderFloat3("Light Translation", (float*)&m_LightTransalations, -30.0f, 30.0f);
		ImGui::SliderFloat3("Light Scale", (float*)&m_LightScale, 0.0f, 3.0f);
		//ImGui::End();
	}

	ImGui::Separator();

	if (ImGui::CollapsingHeader("Ground")) {
		//ImGui::Begin("Editing");
		ImGui::SliderFloat3("Ground Translation", (float*)&m_GroundTransalations, -5.0f, 5.0f);
		ImGui::SliderFloat3("Ground Scale", (float*)&m_GroundScale, 0.0f, 3.0f);
		//ImGui::End();
	}

	ImGui::Separator();

	ImGui::ColorEdit3("Clear Color", (float*)&m_Color);
	ImGui::SliderFloat("Ambient Light", &m_AmbLight, 0.0f, 1.0f);
	ImGui::Text("Framerate: %.f", ImGui::GetIO().Framerate);
	ImGui::Checkbox("V Sync ", &(app.getVSync()));

	ImGui::End();
}