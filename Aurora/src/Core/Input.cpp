#include "Aurorapch.h"
#include "Input.h"

#include "KeyCodes.h"
#include "Core/Application.h"

#include <GLFW/glfw3.h>

namespace Aurora {

	bool Input::isKeyPressed(int keycode)
	{
		GLFWwindow* window = (GLFWwindow*)Application::GetApp().GetWindow().GetWindowPointer();

		int state = glfwGetKey(window, keycode);

		return state == Key::Press || state == Key::Repeat;
	}

	bool Input::isMouseButtonPressed(int button)
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

}