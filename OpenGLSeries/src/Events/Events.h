#pragma once

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

#define EVENT_CLASS_TYPE(type) static EventType getStaticType() { return EventType::type; }\
								virtual EventType getEventType() const override { return getStaticType(); }\
								virtual const char* getName() const override { return #type; };
class Event
{
public:
	virtual ~Event() = default;

	virtual EventType getEventType() const = 0;
	virtual const char* getName() const = 0;
	virtual std::string toString() const { return getName(); }

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