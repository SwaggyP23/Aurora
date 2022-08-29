#include "Aurorapch.h"
#include "Input.h"

#include "Core/Application.h"

#include <glfw/glfw3.h>
#include <imgui/imgui_internal.h>

namespace Aurora {

	bool Input::IsKeyPressed(KeyCode keycode)
	{
		if (!(Application::GetApp().GetSpecification().EnableImGui))
		{
			GLFWwindow* window = (GLFWwindow*)Application::GetApp().GetWindow().GetWindowPointer();

			int state = glfwGetKey(window, (int)keycode);
			
			return state == GLFW_PRESS || state == GLFW_REPEAT;
		}

		ImGuiContext* context = ImGui::GetCurrentContext();
		bool pressed = false;
		for (ImGuiViewport* viewport : context->Viewports)
		{
			if (!viewport->PlatformUserData)
				continue;

			GLFWwindow* windowHandle = *(GLFWwindow**)viewport->PlatformUserData;
			if (!windowHandle)
				continue;

			auto state = glfwGetKey(windowHandle, (int)keycode);
			if (state == GLFW_PRESS || state == GLFW_REPEAT)
			{
				pressed = true;
				break;
			}
		}

		return pressed;
	}

	bool Input::IsMouseButtonPressed(MouseButton mouseCode)
	{
		if (!(Application::GetApp().GetSpecification().EnableImGui))
		{
			GLFWwindow* window = (GLFWwindow*)Application::GetApp().GetWindow().GetWindowPointer();

			int state = glfwGetMouseButton(window, (int)mouseCode);

			return state == GLFW_PRESS || state == GLFW_REPEAT;
		}

		ImGuiContext* context = ImGui::GetCurrentContext();
		bool pressed = false;
		for (ImGuiViewport* viewport : context->Viewports)
		{
			if (!viewport->PlatformUserData)
				continue;

			GLFWwindow* windowHandle = *(GLFWwindow**)viewport->PlatformUserData;
			if (!windowHandle)
				continue;

			auto state = glfwGetMouseButton(windowHandle, (int)mouseCode);
			if (state == GLFW_PRESS || state == GLFW_REPEAT)
			{
				pressed = true;
				break;
			}
		}

		return pressed;
	}

	std::pair<float, float> Input::GetMousePosition()
	{
		GLFWwindow* window = (GLFWwindow*)Application::GetApp().GetWindow().GetWindowPointer();
		double x, y;
		glfwGetCursorPos(window, &x, &y);

		return { (float)x, (float)y };
	}

	float Input::GetMouseX()
	{
		auto [x, y] = GetMousePosition();

		return x;
	}

	float Input::GetMouseY()
	{
		auto [x, y] = GetMousePosition();

		return y;
	}

	void Input::SetCursorMode(CursorMode mode)
	{
		GLFWwindow* window = Application::GetApp().GetWindow().GetWindowPointer();
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL + (int)mode);
	}

	CursorMode Input::GetCursorMode()
	{
		GLFWwindow* window = Application::GetApp().GetWindow().GetWindowPointer();
		return (CursorMode)(glfwGetInputMode(window, GLFW_CURSOR) - GLFW_CURSOR_NORMAL);
	}

}