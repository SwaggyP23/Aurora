#pragma once

#include "KeyCodes.h"
#include "MouseCodes.h"

#include <utility>

namespace Aurora {

	enum class CursorMode : uint8_t
	{
		Normal = 0,
		Hidden,
		Locked
	};

	class Input
	{
	public:
		static bool IsKeyPressed(KeyCode keycode);
		static bool IsMouseButtonPressed(MouseCode button);

		static std::pair<float, float> GetMousePosition();
		static float GetMouseX();
		static float GetMouseY();

		static void SetCursorMode(CursorMode mode);
		static CursorMode GetCursorMode();

	};

}