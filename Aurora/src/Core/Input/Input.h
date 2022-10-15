#pragma once

#include "Core/Base.h"
#include "GamePadCodes.h"

#include <utility>
#include <map>

namespace Aurora {

	enum class CursorMode : uint8_t
	{
		Normal = 0,
		Hidden,
		Locked
	};

	struct KeyData
	{
		KeyCode Key;
		KeyState State = KeyState::None;
		KeyState OldState = KeyState::None;
	};

	struct Controller
	{
		int ID;
		std::string Name;
		std::map<int, bool> ButtonStates;
		std::map<int, float> AxisStates;
		std::map<int, uint8_t> HatStates;
	};

	class Input
	{
	public:
		// Key
		[[nodiscard]] static bool IsKeyPressed(KeyCode keyCode);
		[[nodiscard]] static bool IsKeyHeld(KeyCode keyCode);
		[[nodiscard]] static bool IsKeyDown(KeyCode keyCode);
		[[nodiscard]] static bool IsKeyReleased(KeyCode keyCode);

		// Mouse
		[[nodiscard]] static bool IsMouseButtonPressed(MouseButton mouseCode);
		[[nodiscard]] static std::pair<float, float> GetMousePosition();
		[[nodiscard]] static float GetMouseX();
		[[nodiscard]] static float GetMouseY();

		// Controller
		[[nodiscard]] static bool IsControllerPresent(int id);
		[[nodiscard]] static std::vector<int> GetConnectedControllerIDs();
		[[nodiscard]] static const Controller* GetController(int id);
		[[nodiscard]] static std::string_view GetControllerName(int id);
		[[nodiscard]] static bool IsControllerButtonPressed(int id, int button);
		[[nodiscard]] static float GetControllerAxis(int id, int axis);
		[[nodiscard]] static uint8_t GetControllerHat(int id, int hat);
		[[nodiscard]] static const std::map<int, Controller>& GetControllers() { return s_Controllers; }

		static void SetCursorMode(CursorMode mode);
		[[nodiscard]] static CursorMode GetCursorMode();

	private:
		static void TransitionPressedKeys();
		static void ClearReleasedKeys();
		static void UpdateKeyState(KeyCode key, KeyState newState);
		static void Update();

	private:
		static std::map<KeyCode, KeyData> s_KeyData;
		static std::map<int, Controller> s_Controllers;

		friend class Application;
		friend class Window;

	};

}