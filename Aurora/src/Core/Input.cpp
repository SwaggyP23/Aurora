#include "Aurorapch.h"
#include "Input.h"

#include "Core/Application.h"

#include <glfw/glfw3.h>

namespace Aurora {

	bool Input::IsKeyPressed(KeyCode keycode)
	{
		GLFWwindow* window = (GLFWwindow*)Application::GetApp().GetWindow().GetWindowPointer();
		
		int state = glfwGetKey(window, keycode);
		
		return state == Key::Press || state == Key::Repeat;
	}

	bool Input::IsMouseButtonPressed(MouseCode button)
	{
		GLFWwindow* window = (GLFWwindow*)Application::GetApp().GetWindow().GetWindowPointer();

		int state = glfwGetMouseButton(window, button);

		return state == Key::Press;
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
		GLFWwindow* window = (GLFWwindow*)Application::GetApp().GetWindow().GetWindowPointer();
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL + (int)mode);
	}

	CursorMode Input::GetCursorMode()
	{
		GLFWwindow* window = (GLFWwindow*)Application::GetApp().GetWindow().GetWindowPointer();
		return (CursorMode)(glfwGetInputMode(window, GLFW_CURSOR) - GLFW_CURSOR_NORMAL);
	}

}