#pragma once

#include "Events.h"

class MouseMovedEvent : public Event
{
public:
	MouseMovedEvent(float mx, float my)
		: m_Mx(mx), m_My(my) {}

	inline float getMouseX() const { return m_Mx; }
	inline float getMouseY() const { return m_My; }

	std::string toString() const override
	{
		std::stringstream ss;
		ss << "Mouse Moved Event: X: " << m_Mx << "  Y: " << m_My;
		return ss.str();
	}

	EVENT_CLASS_TYPE(MouseMoved)

private:
	float m_Mx, m_My;

};

class MouseScrolledEvent : public Event
{
public:
	MouseScrolledEvent(float xoff, float yoff)
		: m_Xoffset(xoff), m_Yoffset(yoff) {}

	inline float getXoffset() const {return m_Xoffset; }
	inline float getYoffset() const {return m_Yoffset; }

	std::string toString() const override
	{
		std::stringstream ss;
		ss << "Mouse Scrolled Event: " << m_Xoffset << '\t' << m_Yoffset;
		return ss.str();
	}

	EVENT_CLASS_TYPE(MouseScrolled)

private:
	float m_Xoffset, m_Yoffset;
};

class MouseButtonEvent : public Event
{
public:
	inline int getButtonCode() const { return m_ButtonCode; }

protected:
	MouseButtonEvent(int buttoncode)
		: m_ButtonCode(buttoncode) {}

	int m_ButtonCode;

};

class MouseButtonPressedEvent : public MouseButtonEvent
{
public:
	MouseButtonPressedEvent(int mouseButton)
		: MouseButtonEvent(mouseButton) {}

	std::string toString() const override
	{
		std::stringstream ss;
		ss << "Mouse Button Pressed Event: " << m_ButtonCode;
		return ss.str();
	}

	EVENT_CLASS_TYPE(MouseButtonPressed)

};

class MouseButtonReleasedEvent : public MouseButtonEvent
{
public:
	MouseButtonReleasedEvent(int mouseButton)
		: MouseButtonEvent(mouseButton) {}

	std::string toString() const override
	{
		std::stringstream ss;
		ss << "Mouse Button Released Event: " << m_ButtonCode;
		return ss.str();
	}

	EVENT_CLASS_TYPE(MouseButtonReleased)

};