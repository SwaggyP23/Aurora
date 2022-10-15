#pragma once

#include "Core/Base.h"
#include "Core/Input/KeyMappingCodes.h"

#include <string>
#include <sstream>

/*
 * Events in Aurora are currently blocking, that is when an event occurs it
 * immediately gets dispatched and must be dealt with right then and there.
 * For the future, a better strategy might be to buffer events in an event bus
 * and process them during the "event" part of the update stage
 */

namespace Aurora {

	enum class EventType
	{
		None = 0,
		AppTick, AppUpdate, AppRender,
		WindowResize, WindowMinimize, WindowMaximize, WindowClose, WindowPathDrop,
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
								virtual EventType GetEventType() const override { return getStaticType(); }\
								virtual const char* GetName() const override { return AR_STRINGIFY_MACRO(type); }

	#define EVENT_CLASS_CATEGORY(category) virtual int GetCategoryFlags() const override { return category; }

	class Event
	{
	public:
		virtual ~Event() = default;

		virtual EventType GetEventType() const = 0;
		virtual const char* GetName() const = 0;
		virtual int GetCategoryFlags() const = 0;
		virtual std::string toString() const { return GetName(); }

		bool IsInCategory(EventCategory category)
		{
			return GetCategoryFlags() & category;
		}

		bool Handled = false;

	};

/*
 * This switched from using std::function to using just normal function pointers which are deduced by the compiler in the
 * dispatch function in the F template argument.
 * The switch is to reduce potential heap allocations done by std::function!
 */

	class EventDispatcher
	{
	public:
		EventDispatcher(Event& e)
			: m_Event(e) {}

		template<typename T, typename Func> // Func is to be deduced by compiler, and it will be a function
		bool Dispatch(const Func& func)
		{
			if (m_Event.GetEventType() == T::getStaticType() && !m_Event.Handled)
			{
				m_Event.Handled = func(*(T*)&m_Event);

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