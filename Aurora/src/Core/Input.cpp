#include "Aurorapch.h"
#include "Input.h"

#include "KeyCodes.h"
#include "Core/Application.h"

#include <GLFW/glfw3.h>

namespace Aurora {

	bool Input::isKeyPressed(int keycode)
	{
		GLFWwindow* window = (GLFWwindow*)Application::getApp().getWindow().getWindowPointer();

		int state = glfwGetKey(window, keycode);

		return state == Key::Press || state == Key::Repeat;
	}

	bool Input::isMouseButtonPressed(int button)
	{
		GLFWwindow* window = (GLFWwindow*)Application::getApp().getWindow().getWindowPointer();

		int state = glfwGetMouseButton(window, button);

		return state == Key::Press;
	}

	std::pair<float, float> Input::getMousePosition()
	{
		GLFWwindow* window = (GLFWwindow*)Application::getApp().getWindow().getWindowPointer();
		double x, y;
		glfwGetCursorPos(window, &x, &y);

		return { (float)x, (float)y };
	}

	float Input::getMouseX()
	{
		auto [x, y] = getMousePosition();

		return x;
	}

	float Input::getMouseY()
	{
		auto [x, y] = getMousePosition();

		return y;
	}

}