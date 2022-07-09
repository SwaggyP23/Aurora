#include "Aurorapch.h"
#include "Input.h"

#include "Core/Application.h"
#include <GLFW/glfw3.h>

bool Input::isKeyPressed(int keycode)
{
	GLFWwindow* window = (GLFWwindow*)Application::getApp().getWindow().getWindowPointer();

	int state = glfwGetKey(window, keycode);

	return state == GLFW_PRESS || state == GLFW_REPEAT;
}

bool Input::isMouseButtonPressed(int button)
{
	GLFWwindow* window = (GLFWwindow*)Application::getApp().getWindow().getWindowPointer();

	int state = glfwGetMouseButton(window, button);

	return state == GLFW_PRESS;
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