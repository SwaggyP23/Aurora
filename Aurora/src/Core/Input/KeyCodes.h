#pragma once

#include <ostream>

namespace Aurora {

	enum class KeyCode : uint16_t
	{
		Space            = 32,
		Apostrophe       = 39,  /* ' */
		Comma            = 44,  /* , */
		Minus            = 45,  /* - */
		Period           = 46,  /* . */
		Slash            = 47,  /* / */

		// Digit keys
		Key0             = 48,
		Key1             = 49,
		Key2             = 50,
		Key3             = 51,
		Key4             = 52,
		Key5             = 53,
		Key6             = 54,
		Key7             = 55,
		Key8             = 56,
		Key9             = 57,

		SemiColon        = 59,  /* ; */
		Equal            = 61,  /* = */

		// Alphabet Keys
		A                = 65,
		B                = 66,
		C                = 67,
		D                = 68,
		E                = 69,
		F                = 70,
		G                = 71,
		H                = 72,
		I                = 73,
		J                = 74,
		K                = 75,
		L                = 76,
		M                = 77,
		N                = 78,
		O                = 79,
		P                = 80,
		Q                = 81,
		R                = 82,
		S                = 83,
		T                = 84,
		U                = 85,
		V                = 86,
		W                = 87,
		X                = 88,
		Y                = 89,
		Z                = 90,

		LeftBracket      = 91,  /* [ */
		BackSlash        = 92,  /* \ */
		RightBracket     = 93,  /* ] */
		GraveAccent      = 96,  /* ` */

		World1           = 161, /* non-US #1 */
		World2           = 162, /* non-US #2 */

		// Function Keys
		Escape           = 256,
		Enter            = 257,
		Tab              = 258,
		Backspace        = 259,
		Insert           = 260,
		Delete           = 261,
		Right            = 262,
		Left             = 263,
		Down             = 264,
		Up               = 265,
		PageUp           = 266,
		PageDown         = 267,
		Home             = 268,
		End              = 269,
		CapsLock         = 280,
		ScrollLock       = 281,
		NumLock          = 282,
		PrintScreen      = 283,
		Pause            = 284,
		F1               = 290,
		F2               = 291,
		F3               = 292,
		F4               = 293,
		F5               = 294,
		F6               = 295,
		F7               = 296,
		F8               = 297,
		F9               = 298,
		F10              = 299,
		F11              = 300,
		F12              = 301,
		F13              = 302,
		F14              = 303,
		F15              = 304,
		F16              = 305,
		F17              = 306,
		F18              = 307,
		F19              = 308,
		F20              = 309,
		F21              = 310,
		F22              = 311,
		F23              = 312,
		F24              = 313,
		F25              = 314,

		// Keypad
		KP0              = 320,
		KP1              = 321,
		KP2              = 322,
		KP3              = 323,
		KP4              = 324,
		KP5              = 325,
		KP6              = 326,
		KP7              = 327,
		KP8              = 328,
		KP9              = 329,
		KPDecimal        = 330,
		KPDivide         = 331,
		KPMultiply       = 332,
		KPSubtract       = 333,
		KPAdd            = 334,
		KPEnter          = 335,
		KPEqual          = 336,

		LeftShift        = 340,
		LeftControl      = 341,
		LeftAlt          = 342,
		LeftSuper        = 343,
		RightShift       = 344,
		RightControl     = 345,
		RightAlt         = 346,
		RightSuper       = 347,
		Menu             = 348
	};

	inline std::ostream& operator<<(std::ostream& stream, KeyCode code)
	{
		stream << (int)code;

		return stream;
	}

	enum class KeyState
	{
		None = -1,
		Pressed,
		Held,
		Released
	};

}

/* Printable keys */
/* Taken from glfw3.h */
#define AR_KEY_SPACE              ::Aurora::KeyCode::Space
#define AR_KEY_APOSTROPHE         ::Aurora::KeyCode::Apostrophe
#define AR_KEY_COMMA              ::Aurora::KeyCode::Comma
#define AR_KEY_MINUS              ::Aurora::KeyCode::Minus
#define AR_KEY_PERIOD             ::Aurora::KeyCode::Period
#define AR_KEY_SLASH              ::Aurora::KeyCode::Slash
#define AR_KEY_0                  ::Aurora::KeyCode::Key0
#define AR_KEY_1                  ::Aurora::KeyCode::Key1
#define AR_KEY_2                  ::Aurora::KeyCode::Key2
#define AR_KEY_3                  ::Aurora::KeyCode::Key3
#define AR_KEY_4                  ::Aurora::KeyCode::Key4
#define AR_KEY_5                  ::Aurora::KeyCode::Key5
#define AR_KEY_6                  ::Aurora::KeyCode::Key6
#define AR_KEY_7                  ::Aurora::KeyCode::Key7
#define AR_KEY_8                  ::Aurora::KeyCode::Key8
#define AR_KEY_9                  ::Aurora::KeyCode::Key9
#define AR_KEY_SEMICOLON          ::Aurora::KeyCode::SemiColon
#define AR_KEY_EQUAL              ::Aurora::KeyCode::Equal
#define AR_KEY_A                  ::Aurora::KeyCode::A
#define AR_KEY_B                  ::Aurora::KeyCode::B
#define AR_KEY_C                  ::Aurora::KeyCode::C
#define AR_KEY_D                  ::Aurora::KeyCode::D
#define AR_KEY_E                  ::Aurora::KeyCode::E
#define AR_KEY_F                  ::Aurora::KeyCode::F
#define AR_KEY_G                  ::Aurora::KeyCode::G
#define AR_KEY_H                  ::Aurora::KeyCode::H
#define AR_KEY_I                  ::Aurora::KeyCode::I
#define AR_KEY_J                  ::Aurora::KeyCode::J
#define AR_KEY_K                  ::Aurora::KeyCode::K
#define AR_KEY_L                  ::Aurora::KeyCode::L
#define AR_KEY_M                  ::Aurora::KeyCode::M
#define AR_KEY_N                  ::Aurora::KeyCode::N
#define AR_KEY_O                  ::Aurora::KeyCode::O
#define AR_KEY_P                  ::Aurora::KeyCode::P
#define AR_KEY_Q                  ::Aurora::KeyCode::Q
#define AR_KEY_R                  ::Aurora::KeyCode::R
#define AR_KEY_S                  ::Aurora::KeyCode::S
#define AR_KEY_T                  ::Aurora::KeyCode::T
#define AR_KEY_U                  ::Aurora::KeyCode::U
#define AR_KEY_V                  ::Aurora::KeyCode::V
#define AR_KEY_W                  ::Aurora::KeyCode::W
#define AR_KEY_X                  ::Aurora::KeyCode::X
#define AR_KEY_Y                  ::Aurora::KeyCode::Y
#define AR_KEY_Z                  ::Aurora::KeyCode::Z
#define AR_KEY_LEFT_BRACKET       ::Aurora::KeyCode::LeftBracket
#define AR_KEY_BACKSLASH          ::Aurora::KeyCode::BackSlash
#define AR_KEY_RIGHT_BRACKET      ::Aurora::KeyCode::RightBracket
#define AR_KEY_GRAVE_ACCENT       ::Aurora::KeyCode::GraveAccent
#define AR_KEY_WORLD_1            ::Aurora::KeyCode::World1
#define AR_KEY_WORLD_2            ::Aurora::KeyCode::World2

// Function
#define AR_KEY_ESCAPE             ::Aurora::KeyCode::Escape
#define AR_KEY_ENTER              ::Aurora::KeyCode::Enter
#define AR_KEY_TAB                ::Aurora::KeyCode::Tab
#define AR_KEY_BACKSPACE          ::Aurora::KeyCode::Backspace
#define AR_KEY_INSERT             ::Aurora::KeyCode::Insert
#define AR_KEY_DELETE             ::Aurora::KeyCode::Delete
#define AR_KEY_RIGHT              ::Aurora::KeyCode::Right
#define AR_KEY_LEFT               ::Aurora::KeyCode::Left
#define AR_KEY_DOWN               ::Aurora::KeyCode::Down
#define AR_KEY_UP                 ::Aurora::KeyCode::Up
#define AR_KEY_PAGE_UP            ::Aurora::KeyCode::PageUp
#define AR_KEY_PAGE_DOWN          ::Aurora::KeyCode::PageDown
#define AR_KEY_HOME               ::Aurora::KeyCode::Home
#define AR_KEY_END                ::Aurora::KeyCode::End
#define AR_KEY_CAPS_LOCK          ::Aurora::KeyCode::CapsLock
#define AR_KEY_SCROLL_LOCK        ::Aurora::KeyCode::ScrollLock
#define AR_KEY_NUM_LOCK           ::Aurora::KeyCode::NumLock
#define AR_KEY_PRINT_SCREEN       ::Aurora::KeyCode::PrintScreen
#define AR_KEY_PAUSE              ::Aurora::KeyCode::Pause
#define AR_KEY_F1                 ::Aurora::KeyCode::F1
#define AR_KEY_F2                 ::Aurora::KeyCode::F2
#define AR_KEY_F3                 ::Aurora::KeyCode::F3
#define AR_KEY_F4                 ::Aurora::KeyCode::F4
#define AR_KEY_F5                 ::Aurora::KeyCode::F5
#define AR_KEY_F6                 ::Aurora::KeyCode::F6
#define AR_KEY_F7                 ::Aurora::KeyCode::F7
#define AR_KEY_F8                 ::Aurora::KeyCode::F8
#define AR_KEY_F9                 ::Aurora::KeyCode::F9
#define AR_KEY_F10                ::Aurora::KeyCode::F10
#define AR_KEY_F11                ::Aurora::KeyCode::F11
#define AR_KEY_F12                ::Aurora::KeyCode::F12
#define AR_KEY_F13                ::Aurora::KeyCode::F13
#define AR_KEY_F14                ::Aurora::KeyCode::F14
#define AR_KEY_F15                ::Aurora::KeyCode::F15
#define AR_KEY_F16                ::Aurora::KeyCode::F16
#define AR_KEY_F17                ::Aurora::KeyCode::F17
#define AR_KEY_F18                ::Aurora::KeyCode::F18
#define AR_KEY_F19                ::Aurora::KeyCode::F19
#define AR_KEY_F20                ::Aurora::KeyCode::F20
#define AR_KEY_F21                ::Aurora::KeyCode::F21
#define AR_KEY_F22                ::Aurora::KeyCode::F22
#define AR_KEY_F23                ::Aurora::KeyCode::F23
#define AR_KEY_F24                ::Aurora::KeyCode::F24
#define AR_KEY_F25                ::Aurora::KeyCode::F25

// Keypad
#define AR_KEY_KP_0               ::Aurora::KeyCode::KP0
#define AR_KEY_KP_1               ::Aurora::KeyCode::KP1
#define AR_KEY_KP_2               ::Aurora::KeyCode::KP2
#define AR_KEY_KP_3               ::Aurora::KeyCode::KP3
#define AR_KEY_KP_4               ::Aurora::KeyCode::KP4
#define AR_KEY_KP_5               ::Aurora::KeyCode::KP5
#define AR_KEY_KP_6               ::Aurora::KeyCode::KP6
#define AR_KEY_KP_7               ::Aurora::KeyCode::KP7
#define AR_KEY_KP_8               ::Aurora::KeyCode::KP8
#define AR_KEY_KP_9               ::Aurora::KeyCode::KP9
#define AR_KEY_KP_DECIMAL         ::Aurora::KeyCode::KPDecimal
#define AR_KEY_KP_DIVIDE          ::Aurora::KeyCode::KPDivide
#define AR_KEY_KP_MULTIPLY        ::Aurora::KeyCode::KPMultiply
#define AR_KEY_KP_SUBTRACT        ::Aurora::KeyCode::KPSubtract
#define AR_KEY_KP_ADD             ::Aurora::KeyCode::KPAdd
#define AR_KEY_KP_ENTER           ::Aurora::KeyCode::KPEnter
#define AR_KEY_KP_EQUAL           ::Aurora::KeyCode::KPEqual

// Controls
#define AR_KEY_LEFT_SHIFT         ::Aurora::KeyCode::LeftShift
#define AR_KEY_LEFT_CONTROL       ::Aurora::KeyCode::LeftControl
#define AR_KEY_LEFT_ALT           ::Aurora::KeyCode::LeftAlt
#define AR_KEY_LEFT_SUPER         ::Aurora::KeyCode::LeftSuper
#define AR_KEY_RIGHT_SHIFT        ::Aurora::KeyCode::RightShift
#define AR_KEY_RIGHT_CONTROL      ::Aurora::KeyCode::RightControl
#define AR_KEY_RIGHT_ALT          ::Aurora::KeyCode::RightAlt
#define AR_KEY_RIGHT_SUPER        ::Aurora::KeyCode::RightSuper
#define AR_KEY_MENU               ::Aurora::KeyCode::Menu