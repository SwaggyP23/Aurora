#pragma once

#include <utility>

class Input
{
public:
	static bool isKeyPressed(int keycode);

	static bool isMouseButtonPressed(int button);
	static std::pair<float, float> getMousePosition();
	static float getMouseX();
	static float getMouseY();

};