#include "OGLpch.h"
#include "InputImpl.h"

#include "Application.h"
#include <GLFW/glfw3.h>

Input* Input::s_Instance = new InputImplementation();

bool InputImplementation::isKeyPressedImpl(int keycode) const
{
	GLFWwindow* window = (GLFWwindow*)Application::getApp().getWindow().getWindowPointer();

	int state = glfwGetKey(window, keycode);

	return state == GLFW_PRESS || state == GLFW_REPEAT;
}

bool InputImplementation::isMouseButtonPressedImpl(int button) const
{
	GLFWwindow* window = (GLFWwindow*)Application::getApp().getWindow().getWindowPointer();

	int state = glfwGetMouseButton(window, button);

	return state == GLFW_PRESS;
}

std::pair<float, float> InputImplementation::getMousePositionImpl() const
{
	GLFWwindow* window = (GLFWwindow*)Application::getApp().getWindow().getWindowPointer();
	double x, y;
	glfwGetCursorPos(window, &x, &y);

	return { (float)x, (float)y };
}

float InputImplementation::getMouseXOffImpl() const
{
	auto[x, y] = getMousePositionImpl();

	return x;
}

float InputImplementation::getMouseYOffImpl() const
{
	auto[x, y] = getMousePositionImpl();

	return y;
}