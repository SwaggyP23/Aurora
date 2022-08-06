#pragma once

#include "Core/Base.h"
#include "Core/KeyCodes.h"
#include "Core/MouseCodes.h"

#include <string>
#include <sstream>

/*
 * This switched from using std::function to using just normal function pointers which are deduced by the compiler in the
 * dispatch function in the F template argument.
 * The switch is to reduce potential heap allocations done by std::function!
 */ 

namespace Aurora {

	enum class EventType
	{
		None = 0,
		WindowResize, WindowMinimize, WindowMaximize, WindowClose,
		KeyPressed, KeyReleased, KeyTyped,
		MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled
	};

	enum EventCategory
	{
		None = 0,
		EventCategoryApplication       = BIT(0),
		EventCategoryInput             = BIT(1),
		EventCategoryKeyboard          = BIT(2),
		EventCategoryMouse             = BIT(3),
		EventCategoryMouseButton       = BIT(4)
	};

	#define EVENT_CLASS_TYPE(type) static EventType getStaticType() { return EventType::type; }\
								virtual EventType getEventType() const override { return getStaticType(); }\
								virtual const char* getName() const override { return AR_STRINGIFY_MACRO(type); };

	#define EVENT_CLASS_CATEGORY(category) virtual int GetCategoryFlags() const override { return category; }

	class Event
	{
	public:
		virtual ~Event() = default;

		virtual EventType getEventType() const = 0;
		virtual const char* getName() const = 0;
		virtual int GetCategoryFlags() const = 0;
		virtual std::string toString() const { return getName(); }

		bool IsInCategory(EventCategory category)
		{
			return GetCategoryFlags() & category;
		}

		bool Handled = false;

	};

	class EventDispatcher
	{
	public:
		EventDispatcher(Event& e)
			: m_Event(e) {}

		template<typename T, typename F> // F is to be deduced by compiler, and it will be a function
		bool dispatch(const F& func)
		{
			if (m_Event.getEventType() == T::getStaticType())
			{
				m_Event.Handled |= func(static_cast<T&>(m_Event));
				return true;
			}
			return false;
		}

	private:
		Event& m_Event;

	};

	inline std::ostream& operator<<(std::ostream& stream, const Event& e)
	{
		return stream << e.toString();
	}

}