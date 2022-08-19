#pragma once

#include <stdint.h>

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

	using Key = KeyCode;

	inline std::ostream& operator<<(std::ostream& stream, KeyCode code)
	{
		stream << (int)code;
		return stream;
	}

}

/* Printable keys */
#define AR_KEY_SPACE              ::Aurora::Key::Space
#define AR_KEY_APOSTROPHE         ::Aurora::Key::Apostrophe
#define AR_KEY_COMMA              ::Aurora::Key::Comma
#define AR_KEY_MINUS              ::Aurora::Key::Minus
#define AR_KEY_PERIOD             ::Aurora::Key::Period
#define AR_KEY_SLASH              ::Aurora::Key::Slash
#define AR_KEY_0                  ::Aurora::Key::Key0
#define AR_KEY_1                  ::Aurora::Key::Key1
#define AR_KEY_2                  ::Aurora::Key::Key2
#define AR_KEY_3                  ::Aurora::Key::Key3
#define AR_KEY_4                  ::Aurora::Key::Key4
#define AR_KEY_5                  ::Aurora::Key::Key5
#define AR_KEY_6                  ::Aurora::Key::Key6
#define AR_KEY_7                  ::Aurora::Key::Key7
#define AR_KEY_8                  ::Aurora::Key::Key8
#define AR_KEY_9                  ::Aurora::Key::Key9
#define AR_KEY_SEMICOLON          ::Aurora::Key::SemiColon
#define AR_KEY_EQUAL              ::Aurora::Key::Equal
#define AR_KEY_A                  ::Aurora::Key::A
#define AR_KEY_B                  ::Aurora::Key::B
#define AR_KEY_C                  ::Aurora::Key::C
#define AR_KEY_D                  ::Aurora::Key::D
#define AR_KEY_E                  ::Aurora::Key::E
#define AR_KEY_F                  ::Aurora::Key::F
#define AR_KEY_G                  ::Aurora::Key::G
#define AR_KEY_H                  ::Aurora::Key::H
#define AR_KEY_I                  ::Aurora::Key::I
#define AR_KEY_J                  ::Aurora::Key::J
#define AR_KEY_K                  ::Aurora::Key::K
#define AR_KEY_L                  ::Aurora::Key::L
#define AR_KEY_M                  ::Aurora::Key::M
#define AR_KEY_N                  ::Aurora::Key::N
#define AR_KEY_O                  ::Aurora::Key::O
#define AR_KEY_P                  ::Aurora::Key::P
#define AR_KEY_Q                  ::Aurora::Key::Q
#define AR_KEY_R                  ::Aurora::Key::R
#define AR_KEY_S                  ::Aurora::Key::S
#define AR_KEY_T                  ::Aurora::Key::T
#define AR_KEY_U                  ::Aurora::Key::U
#define AR_KEY_V                  ::Aurora::Key::V
#define AR_KEY_W                  ::Aurora::Key::W
#define AR_KEY_X                  ::Aurora::Key::X
#define AR_KEY_Y                  ::Aurora::Key::Y
#define AR_KEY_Z                  ::Aurora::Key::Z
#define AR_KEY_LEFT_BRACKET       ::Aurora::Key::LeftBracket
#define AR_KEY_BACKSLASH          ::Aurora::Key::BackSlash
#define AR_KEY_RIGHT_BRACKET      ::Aurora::Key::RightBracket
#define AR_KEY_GRAVE_ACCENT       ::Aurora::Key::GraveAccent
#define AR_KEY_WORLD_1            ::Aurora::Key::World1
#define AR_KEY_WORLD_2            ::Aurora::Key::World2

// Function
#define AR_KEY_ESCAPE             ::Aurora::Key::Escape
#define AR_KEY_ENTER              ::Aurora::Key::Enter
#define AR_KEY_TAB                ::Aurora::Key::Tab
#define AR_KEY_BACKSPACE          ::Aurora::Key::Backspace
#define AR_KEY_INSERT             ::Aurora::Key::Insert
#define AR_KEY_DELETE             ::Aurora::Key::Delete
#define AR_KEY_RIGHT              ::Aurora::Key::Right
#define AR_KEY_LEFT               ::Aurora::Key::Left
#define AR_KEY_DOWN               ::Aurora::Key::Down
#define AR_KEY_UP                 ::Aurora::Key::Up
#define AR_KEY_PAGE_UP            ::Aurora::Key::PageUp
#define AR_KEY_PAGE_DOWN          ::Aurora::Key::PageDown
#define AR_KEY_HOME               ::Aurora::Key::Home
#define AR_KEY_END                ::Aurora::Key::End
#define AR_KEY_CAPS_LOCK          ::Aurora::Key::CapsLock
#define AR_KEY_SCROLL_LOCK        ::Aurora::Key::ScrollLock
#define AR_KEY_NUM_LOCK           ::Aurora::Key::NumLock
#define AR_KEY_PRINT_SCREEN       ::Aurora::Key::PrintScreen
#define AR_KEY_PAUSE              ::Aurora::Key::Pause
#define AR_KEY_F1                 ::Aurora::Key::F1
#define AR_KEY_F2                 ::Aurora::Key::F2
#define AR_KEY_F3                 ::Aurora::Key::F3
#define AR_KEY_F4                 ::Aurora::Key::F4
#define AR_KEY_F5                 ::Aurora::Key::F5
#define AR_KEY_F6                 ::Aurora::Key::F6
#define AR_KEY_F7                 ::Aurora::Key::F7
#define AR_KEY_F8                 ::Aurora::Key::F8
#define AR_KEY_F9                 ::Aurora::Key::F9
#define AR_KEY_F10                ::Aurora::Key::F10
#define AR_KEY_F11                ::Aurora::Key::F11
#define AR_KEY_F12                ::Aurora::Key::F12
#define AR_KEY_F13                ::Aurora::Key::F13
#define AR_KEY_F14                ::Aurora::Key::F14
#define AR_KEY_F15                ::Aurora::Key::F15
#define AR_KEY_F16                ::Aurora::Key::F16
#define AR_KEY_F17                ::Aurora::Key::F17
#define AR_KEY_F18                ::Aurora::Key::F18
#define AR_KEY_F19                ::Aurora::Key::F19
#define AR_KEY_F20                ::Aurora::Key::F20
#define AR_KEY_F21                ::Aurora::Key::F21
#define AR_KEY_F22                ::Aurora::Key::F22
#define AR_KEY_F23                ::Aurora::Key::F23
#define AR_KEY_F24                ::Aurora::Key::F24
#define AR_KEY_F25                ::Aurora::Key::F25

// Keypad
#define AR_KEY_KP_0               ::Aurora::Key::KP0
#define AR_KEY_KP_1               ::Aurora::Key::KP1
#define AR_KEY_KP_2               ::Aurora::Key::KP2
#define AR_KEY_KP_3               ::Aurora::Key::KP3
#define AR_KEY_KP_4               ::Aurora::Key::KP4
#define AR_KEY_KP_5               ::Aurora::Key::KP5
#define AR_KEY_KP_6               ::Aurora::Key::KP6
#define AR_KEY_KP_7               ::Aurora::Key::KP7
#define AR_KEY_KP_8               ::Aurora::Key::KP8
#define AR_KEY_KP_9               ::Aurora::Key::KP9
#define AR_KEY_KP_DECIMAL         ::Aurora::Key::KPDecimal
#define AR_KEY_KP_DIVIDE          ::Aurora::Key::KPDivide
#define AR_KEY_KP_MULTIPLY        ::Aurora::Key::KPMultiply
#define AR_KEY_KP_SUBTRACT        ::Aurora::Key::KPSubtract
#define AR_KEY_KP_ADD             ::Aurora::Key::KPAdd
#define AR_KEY_KP_ENTER           ::Aurora::Key::KPEnter
#define AR_KEY_KP_EQUAL           ::Aurora::Key::KPEqual

// Controls
#define AR_KEY_LEFT_SHIFT         ::Aurora::Key::LeftShift
#define AR_KEY_LEFT_CONTROL       ::Aurora::Key::LeftControl
#define AR_KEY_LEFT_ALT           ::Aurora::Key::LeftAlt
#define AR_KEY_LEFT_SUPER         ::Aurora::Key::LeftSuper
#define AR_KEY_RIGHT_SHIFT        ::Aurora::Key::RightShift
#define AR_KEY_RIGHT_CONTROL      ::Aurora::Key::RightControl
#define AR_KEY_RIGHT_ALT          ::Aurora::Key::RightAlt
#define AR_KEY_RIGHT_SUPER        ::Aurora::Key::RightSuper
#define AR_KEY_MENU               ::Aurora::Key::Menu

#define AR_KEY_LAST               GLFW_KEY_MENU