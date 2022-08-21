#pragma once

#include "Events.h"

namespace Aurora {

	class WindowResizeEvent : public Event
	{
	public:
		WindowResizeEvent(uint32_t width, uint32_t height)
			: m_Width(width), m_Height(height) {}

		inline uint32_t GetWidth() const { return m_Width; }
		inline uint32_t GetHeight() const { return m_Height; }

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

	class WindowMinimizeEvent : public Event
	{
	public:
		WindowMinimizeEvent(bool minimized)
			: m_Minimized(minimized) {}

		bool IsMinimized() const { return m_Minimized; }

		std::string toString() const override
		{
			std::stringstream ss;
			ss << "Window Minimize Event!";
			return ss.str();
		}

		EVENT_CLASS_TYPE(WindowMinimize)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)

	private:
		bool m_Minimized = false;

	};

	class WindowMaximizeEvent : public Event
	{
	public:
		WindowMaximizeEvent() = default;

		std::string toString() const override
		{
			std::stringstream ss;
			ss << "Window Maximize Event!";
			return ss.str();
		}

		EVENT_CLASS_TYPE(WindowMaximize)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)

	};

	class WindowCloseEvent : public Event
	{
	public:
		WindowCloseEvent() = default;

		std::string toString() const override
		{
			std::stringstream ss;
			ss << "Window Close Event!";
			return ss.str();
		}

		EVENT_CLASS_TYPE(WindowClose)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)

	};

	class AppTickEvent : public Event
	{
	public:
		AppTickEvent() = default;

		std::string toString() const override
		{
			std::stringstream ss;
			ss << "App Tick Event!";
			return ss.str();
		}

		EVENT_CLASS_TYPE(AppTick)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)

	};

	class AppUpdateEvent : public Event
	{
	public:
		AppUpdateEvent() = default;

		std::string toString() const override
		{
			std::stringstream ss;
			ss << "App Update Event!";
			return ss.str();
		}

		EVENT_CLASS_TYPE(AppUpdate)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)

	};

	class AppRenderEvent : public Event
	{
	public:
		AppRenderEvent() = default;

		std::string toString() const override
		{
			std::stringstream ss;
			ss << "App Render Event!";
			return ss.str();
		}

		EVENT_CLASS_TYPE(AppRender)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)

	};

}