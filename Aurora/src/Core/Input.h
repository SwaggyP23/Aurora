#pragma once

#include "KeyCodes.h"
#include "MouseCodes.h"

#include "Base.h"

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
		[[nodiscard]] static bool IsKeyPressed(KeyCode keycode);
		[[nodiscard]] static bool IsMouseButtonPressed(MouseButton mouseCode);

		[[nodiscard]] static std::pair<float, float> GetMousePosition();
		[[nodiscard]] static float GetMouseX();
		[[nodiscard]] static float GetMouseY();

		static void SetCursorMode(CursorMode mode);
		[[nodiscard]] static CursorMode GetCursorMode();

	};

}