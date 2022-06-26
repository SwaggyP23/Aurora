#pragma once

#include "Events.h"

class WindowResizeEvent : public Event
{
public:
	WindowResizeEvent(unsigned int width, unsigned int height)
		: m_Width(width), m_Height(height) {}

	inline unsigned int getWidth() const { return m_Width; }
	inline unsigned int getHeight() const { return m_Height; }

	std::string toString() const override
	{
		std::stringstream ss;
		ss << "Window Resize Event: " << m_Width << '\t' << m_Height;
		return ss.str();
	}

	EVENT_CLASS_TYPE(WindowResize)

private:
	unsigned int m_Width;
	unsigned int m_Height;
};

class WindowCloseEvent : public Event
{
public:
	WindowCloseEvent() = default;

	EVENT_CLASS_TYPE(WindowClose)
};