#pragma once

#include "Core/Base.h"

#include <string>
#include <functional>
#include <sstream>

enum class EventType
{
	None = 0,
	WindowResize, WindowClose,
	KeyPressed, KeyReleased, KeyTyped,
	MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled
};

enum EventCategory
{
	None = 0,
	EventCategoryApplication = BIT(0),
	EventCategoryInput = BIT(1),
	EventCategoryKeyboard = BIT(2),
	EventCategoryMouse = BIT(3),
	EventCategoryMouseButton = BIT(4)
};

#define EVENT_CLASS_TYPE(type) static EventType getStaticType() { return EventType::type; }\
								virtual EventType getEventType() const override { return getStaticType(); }\
								virtual const char* getName() const override { return #type; };

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
	template<typename T>
	using EventFn = std::function<bool(T&)>; // This is a function that returns a bool and takes T& arg.
											 // Also T here is the type of even e.g. WindowResizeEvent...
public:
	EventDispatcher(Event& e)
		: m_Event(e) {}

	template<typename T>
	bool dispatch(EventFn<T> func)
	{
		if (m_Event.getEventType() == T::getStaticType())
		{
			m_Event.Handled = func(*(T*)&m_Event);
			return true;
		}
		return false;
	}

private:
	Event& m_Event;

};

inline std::ostream& operator<<(std::ostream & stream, const Event & e)
{
	return stream << e.toString();
}