#pragma once

#include "Events.h"

class WindowResizeEvent : public Event
{
public:
	WindowResizeEvent(uint32_t width, uint32_t height)
		: m_Width(width), m_Height(height) {}

	inline uint32_t getWidth() const { return m_Width; }
	inline uint32_t getHeight() const { return m_Height; }

	std::string toString() const override
	{
		std::stringstream ss;
		ss << "Window Resize Event: " << m_Width << '\t' << m_Height;
		return ss.str();
	}

	EVENT_CLASS_TYPE(WindowResize)
	EVENT_CLASS_CATEGORY(EventCategoryApplication)

private:
	uint32_t m_Width;
	uint32_t m_Height;
};

class WindowCloseEvent : public Event
{
public:
	WindowCloseEvent() = default;

	EVENT_CLASS_TYPE(WindowClose)
	EVENT_CLASS_CATEGORY(EventCategoryApplication)
};