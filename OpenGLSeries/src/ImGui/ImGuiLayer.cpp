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
	ImGui::CreateContext();
	ImGui::StyleColorsDark();

	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
	io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
	io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;

	
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

void ImGuiLayer::onUpdate()
{
	ImGuiIO& io = ImGui::GetIO();
	Application& app = Application::getApp();
	io.DisplaySize = ImVec2(app.getWindow().getWidth(), app.getWindow().getHeight());

	float delta = (float)glfwGetTime();
	io.DeltaTime = m_Time > 0.0f ? (delta - m_Time) : (1/ 60.0f);
	m_Time = delta;

	ImGui_ImplGlfw_NewFrame();
	ImGui_ImplOpenGL3_NewFrame();
	ImGui::NewFrame();

	ImGui::Begin("Colors");
	//ImGui::ColorEdit3("Clear Color:", (float*)&color);
	//ImGui::ColorEdit3("Uniform Color:", (float*)&uniColor);
	//ImGui::SliderFloat("FOV:", &(m_Camera->getZoom()), 10.0f, 90.0f);
	ImGui::SliderFloat("Blend:", &blend, 0.0f, 1.0f);
	//ImGui::SliderFloat("rotation:", &rotation, -10.0f, 10.0f);
	//ImGui::SliderFloat3("transforms:", &translation[0], 10.0f, -5.0f);
	ImGui::Text("Average FrameRate: %.3f", ImGui::GetIO().Framerate);
	ImGui::End();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void ImGuiLayer::onEvent(Event& e)
{
	EventDispatcher dispatcher(e);
	dispatcher.dispatch<MouseMovedEvent>(SET_EVENT_FN(ImGuiLayer::onMouseMovedEvent));
	dispatcher.dispatch<MouseScrolledEvent>(SET_EVENT_FN(ImGuiLayer::onMouseScrolledEvent));
	dispatcher.dispatch<MouseButtonPressedEvent>(SET_EVENT_FN(ImGuiLayer::onMouseButtonPressedEvent));
	dispatcher.dispatch<MouseButtonReleasedEvent>(SET_EVENT_FN(ImGuiLayer::onMouseButtonReleasedEvent));
	dispatcher.dispatch<KeyPressedEvent>(SET_EVENT_FN(ImGuiLayer::onKeyPressedEvent));
	dispatcher.dispatch<KeyReleasedEvent>(SET_EVENT_FN(ImGuiLayer::onKeyReleasedEvent));
	dispatcher.dispatch<KeyTypedEvent>(SET_EVENT_FN(ImGuiLayer::onKeyTypedEvent));
	dispatcher.dispatch<WindowResizeEvent>(SET_EVENT_FN(ImGuiLayer::onWindowResizeEvent));
}

bool ImGuiLayer::onMouseMovedEvent(MouseMovedEvent& e)
{
	ImGuiIO& io = ImGui::GetIO();
	io.MousePos = ImVec2(e.getMouseX(), e.getMouseY());

	return false;
}

bool ImGuiLayer::onMouseScrolledEvent(MouseScrolledEvent& e)
{
	ImGuiIO& io = ImGui::GetIO();
	io.MouseWheel += e.getYoffset();

	return false;
}

bool ImGuiLayer::onMouseButtonPressedEvent(MouseButtonPressedEvent& e)
{
	ImGuiIO& io = ImGui::GetIO();
	io.MouseDown[e.getButtonCode()] = true;

	return false;
}

bool ImGuiLayer::onMouseButtonReleasedEvent(MouseButtonReleasedEvent& e)
{
	ImGuiIO& io = ImGui::GetIO();
	io.MouseDown[e.getButtonCode()] = false;

	return false;
}

bool ImGuiLayer::onKeyPressedEvent(KeyPressedEvent& e)
{
	ImGuiIO& io = ImGui::GetIO();
	io.KeysDown[e.getKeyCode()] = true;

	return false;
}

bool ImGuiLayer::onKeyReleasedEvent(KeyReleasedEvent& e)
{
	ImGuiIO& io = ImGui::GetIO();
	io.KeysDown[e.getKeyCode()] = false;

	io.KeyCtrl = io.KeysDown[GLFW_KEY_LEFT_CONTROL] || io.KeysDown[GLFW_KEY_RIGHT_CONTROL];
	io.KeyShift = io.KeysDown[GLFW_KEY_LEFT_SHIFT] || io.KeysDown[GLFW_KEY_RIGHT_SHIFT];
	io.KeyAlt = io.KeysDown[GLFW_KEY_LEFT_ALT] || io.KeysDown[GLFW_KEY_RIGHT_ALT];
	io.KeySuper = io.KeysDown[GLFW_KEY_LEFT_SUPER] || io.KeysDown[GLFW_KEY_RIGHT_SUPER];

	return false;
}

bool ImGuiLayer::onKeyTypedEvent(KeyTypedEvent& e)
{
	ImGuiIO& io = ImGui::GetIO();
	int keycode = e.getKeyCode();
	if (keycode > 0 && keycode < 0x100000)
		io.AddInputCharacter((unsigned short)keycode);

	return false;
}

bool ImGuiLayer::onWindowResizeEvent(WindowResizeEvent& e)
{
	ImGuiIO& io = ImGui::GetIO();
	io.DisplaySize = ImVec2(e.getWidth(), e.getHeight());
	io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);

	return false;
}