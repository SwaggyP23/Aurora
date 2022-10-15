#pragma once

#include <ostream>

namespace Aurora {

	enum class MouseButton : uint8_t
	{
		Button0          = 0,
		Button1          = 1,
		Button2          = 2,
		Button3          = 3,
		Button4          = 4,
		Button5          = 5,
		Button6          = 6,
		Button7          = 7,

		ButtonLeft       = Button0,
		ButtonRight      = Button1,
		ButtonMiddle     = Button2
	};

	inline std::ostream& operator<<(std::ostream& stream, MouseButton code)
	{
		stream << (int)code;

		return stream;
	}

}

#define AR_MOUSE_BUTTON_0        ::Aurora::MouseButton::Button0
#define AR_MOUSE_BUTTON_1        ::Aurora::MouseButton::Button1
#define AR_MOUSE_BUTTON_2        ::Aurora::MouseButton::Button2
#define AR_MOUSE_BUTTON_3        ::Aurora::MouseButton::Button3
#define AR_MOUSE_BUTTON_4        ::Aurora::MouseButton::Button4
#define AR_MOUSE_BUTTON_5        ::Aurora::MouseButton::Button5
#define AR_MOUSE_BUTTON_6        ::Aurora::MouseButton::Button6
#define AR_MOUSE_BUTTON_7        ::Aurora::MouseButton::Button7

#define AR_MOUSE_BUTTON_LEFT     ::Aurora::MouseButton::ButtonLeft
#define AR_MOUSE_BUTTON_RIGHT    ::Aurora::MouseButton::ButtonRight
#define AR_MOUSE_BUTTON_MIDDLE   ::Aurora::MouseButton::ButtonMiddle