#pragma once

#include "Events.h"

namespace Aurora {

	class WindowResizeEvent : public Event
	{
	public:
		WindowResizeEvent(uint32_t width, uint32_t height)
			: m_Width(width), m_Height(height) {}

		[[nodiscard]] inline uint32_t GetWidth() const { return m_Width; }
		[[nodiscard]] inline uint32_t GetHeight() const { return m_Height; }

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

		[[nodiscard]] bool IsMinimized() const { return m_Minimized; }

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

	class WindowPathDropEvent : public Event
	{
	public:
		using paths = std::vector<std::filesystem::path>;

	public:
		WindowPathDropEvent(uint32_t pathCount, const char** paths)
			: m_PathCount(pathCount)
		{
			// Performing deep copy
			m_Paths.reserve(pathCount);
			for (uint32_t i = 0; i < pathCount; i++)
				m_Paths.emplace_back(std::string(paths[i]));
		}

		[[nodiscard]] inline const paths& GetDroppedPaths() const { return m_Paths; }
		[[nodiscard]] inline uint32_t GetDroppedPathCount() const { return m_PathCount; }

		std::string toString() const override
		{
			std::stringstream ss;
			ss << "Window Path Drop Event!";
			return ss.str();
		}

		EVENT_CLASS_TYPE(WindowPathDrop)
		EVENT_CLASS_CATEGORY(EventCategoryInput)

	private:
		paths m_Paths;
		uint32_t m_PathCount = 0;

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