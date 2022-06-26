#pragma once

#include "Events.h"

class KeyEvent : public Event
{
public:
	inline int getKeyCode() const { return m_KeyCode; }

protected:
	KeyEvent(int keycode)
		: m_KeyCode(keycode) {}

	int m_KeyCode;
};

class KeyPressedEvent : public KeyEvent
{
public:
	KeyPressedEvent(int keycode, int repeatCount)
		: KeyEvent(keycode), m_RepeatCount(repeatCount) {}

	inline int getRepeatCount() const { return m_RepeatCount; }

	std::string toString() const override
	{
		std::stringstream ss;
		ss << "Key Pressed Event: " << m_KeyCode << " (" << m_RepeatCount << ")";
		return ss.str();
	}

	EVENT_CLASS_TYPE(KeyPressed)

private:
	int m_RepeatCount;

};

class KeyReleasedEvent : public KeyEvent
{
public:
	KeyReleasedEvent(int keycode)
		: KeyEvent(keycode) {}

	std::string toString() const override
	{
		std::stringstream ss;
		ss << "Key Released Event: " << m_KeyCode;
		return ss.str();
	}

	EVENT_CLASS_TYPE(KeyReleased)

};

class KeyTypedEvent : public KeyEvent
{
public:
	KeyTypedEvent(int keycode)
		: KeyEvent(keycode) {}

	std::string toString() const override
	{
		std::stringstream ss;
		ss << "Key Typed Event: " << m_KeyCode;
		return ss.str();
	}

	EVENT_CLASS_TYPE(KeyTyped)

};