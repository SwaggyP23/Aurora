#pragma once

#include <utility>

namespace Aurora {

	class Input
	{
	public:
		static bool isKeyPressed(int keycode);

		static bool isMouseButtonPressed(int button);
		static std::pair<float, float> getMousePosition();
		static float getMouseX();
		static float getMouseY();

	};

}