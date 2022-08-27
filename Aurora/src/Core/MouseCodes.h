#pragma once

#include <stdint.h>

namespace Aurora {

	enum class MouseCode : uint16_t
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

	inline std::ostream& operator<<(std::ostream& stream, MouseCode code)
	{
		stream << (int)code;
		return stream;
	}

}

#define AR_MOUSE_BUTTON_0        ::Aurora::MouseCode::Button0
#define AR_MOUSE_BUTTON_1        ::Aurora::MouseCode::Button1
#define AR_MOUSE_BUTTON_2        ::Aurora::MouseCode::Button2
#define AR_MOUSE_BUTTON_3        ::Aurora::MouseCode::Button3
#define AR_MOUSE_BUTTON_4        ::Aurora::MouseCode::Button4
#define AR_MOUSE_BUTTON_5        ::Aurora::MouseCode::Button5
#define AR_MOUSE_BUTTON_6        ::Aurora::MouseCode::Button6
#define AR_MOUSE_BUTTON_7        ::Aurora::MouseCode::Button7

#define AR_MOUSE_BUTTON_LEFT     ::Aurora::MouseCode::ButtonLeft
#define AR_MOUSE_BUTTON_RIGHT    ::Aurora::MouseCode::ButtonRight
#define AR_MOUSE_BUTTON_MIDDLE   ::Aurora::MouseCode::ButtonMiddle