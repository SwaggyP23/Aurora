#pragma once

#include "Events.h"

namespace Aurora {

	class MouseMovedEvent : public Event
	{
	public:
		MouseMovedEvent(float mx, float my)
			: m_Mx(mx), m_My(my) {}

		inline std::pair<float, float> GetMousePos() const { return { m_Mx, m_My }; }
		inline float GetMouseX() const { return m_Mx; }
		inline float GetMouseY() const { return m_My; }

		std::string toString() const override
		{
			std::stringstream ss;
			ss << "Mouse Moved Event: X: " << m_Mx << "  Y: " << m_My;
			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseMoved)
		EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)

	private:
		float m_Mx, m_My;

	};

	class MouseScrolledEvent : public Event
	{
	public:
		MouseScrolledEvent(float xoff, float yoff)
			: m_Xoffset(xoff), m_Yoffset(yoff) {}

		inline float GetXOffset() const { return m_Xoffset; }
		inline float GetYOffset() const { return m_Yoffset; }

		std::string toString() const override
		{
			std::stringstream ss;
			ss << "Mouse Scrolled Event: " << m_Xoffset << '\t' << m_Yoffset;
			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseScrolled)
		EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)

	private:
		float m_Xoffset, m_Yoffset;

	};

	class MouseButtonEvent : public Event
	{
	public:
		virtual ~MouseButtonEvent() = default;

		inline MouseCode GetButtonCode() const { return m_ButtonCode; }

		EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput | EventCategoryMouseButton)

	protected:
		MouseButtonEvent(MouseCode buttoncode)
			: m_ButtonCode(buttoncode) {}

		MouseCode m_ButtonCode;

	};

	class MouseButtonPressedEvent : public MouseButtonEvent
	{
	public:
		MouseButtonPressedEvent(MouseCode mouseButton)
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
		MouseButtonReleasedEvent(MouseCode mouseButton)
			: MouseButtonEvent(mouseButton) {}

		std::string toString() const override
		{
			std::stringstream ss;
			ss << "Mouse Button Released Event: " << m_ButtonCode;
			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseButtonReleased)

	};

}