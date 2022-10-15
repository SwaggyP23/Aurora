#pragma once

#include <ostream>

namespace Aurora {

	enum class GamepadButton : uint8_t
	{
		Button0        = 0, // A (Xbox)
		Button1        = 1, // B (Xbox)
		Button2        = 2, // C (Xbox)
		Button3        = 3, // D (Xbox)

		// Xbox...
		A              = Button0,
		B              = Button1,
		X              = Button2,
		Y              = Button3,

		// Playstation...
		Cross          = Button0,
		Circle         = Button1,
		Square         = Button2,
		Triangle       = Button3,

		LeftBumper     = 4,
		RightBumper    = 5,

		Start          = 6,
		Guide          = 7,

		LeftJoystick   = 8,
		RightJoystick  = 9,

		DPadUp         = 10,
		DPadRight      = 11,
		DPadDown       = 12,
		DPadLeft       = 13
	};

	inline std::ostream& operator<<(std::ostream& stream, GamepadButton buttonCode)
	{
		stream << (int)buttonCode;
		
		return stream;
	}

}

// Xbox
#define AR_GAMEPAD_BUTTON_A              ::Aurora::GamepadButton::A
#define AR_GAMEPAD_BUTTON_B				 ::Aurora::GamepadButton::B
#define AR_GAMEPAD_BUTTON_X				 ::Aurora::GamepadButton::X
#define AR_GAMEPAD_BUTTON_Y				 ::Aurora::GamepadButton::Y

// Playstation
#define AR_GAMEPAD_BUTTON_CROSS			 ::Aurora::GamepadButton::Cross
#define AR_GAMEPAD_BUTTON_CIRCLE		 ::Aurora::GamepadButton::Circle
#define AR_GAMEPAD_BUTTON_SQUARE		 ::Aurora::GamepadButton::Square
#define AR_GAMEPAD_BUTTON_TRIANGLE		 ::Aurora::GamepadButton::Traingle

#define AR_GAMEPAD_LEFT_BUMPER			 ::Aurora::GamepadButton::LeftBumper
#define AR_GAMEPAD_RIGHT_BUMPER			 ::Aurora::GamepadButton::RightBumper

#define AR_GAMEPAD_BUTTON_START			 ::Aurora::GamepadButton::Start
#define AR_GAMEPAD_BUTTON_GUIDE			 ::Aurora::GamepadButton::Guide

#define AR_GAMEPAD_LEFT_JOYSTICK		 ::Aurora::GamepadButton::LeftJoystick
#define AR_GAMEPAD_RIGHT_JOYSTICK		 ::Aurora::GamepadButton::RightJoystick

#define AR_GAMEPAD_DPAD_UP				 ::Aurora::GamepadButton::DPadUp
#define AR_GAMEPAD_DPAD_RIGHT			 ::Aurora::GamepadButton::DPadRight
#define AR_GAMEPAD_DPAD_DOWN			 ::Aurora::GamepadButton::DPadDown
#define AR_GAMEPAD_DPAD_LEFT			 ::Aurora::GamepadButton::DPadLeft