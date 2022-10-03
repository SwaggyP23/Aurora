#pragma once

#include "Core/TimeStep.h"
#include "Events/Events.h"

#include <string>

namespace Aurora {

	class Layer // This is an interface
	{
	public:
		Layer(const std::string& name = "Default Layer");
		virtual ~Layer() = default;

		virtual void OnAttach() {}
		virtual void OnDetach() {}
		virtual void OnUpdate(TimeStep ts) {}
		virtual void OnTick() {}
		virtual void OnImGuiRender() {}
		virtual void OnEvent(Event& e) {}

		inline const std::string& GetDebugName() const { return m_DebugName; }
		inline void SetDebugName(const std::string& name) { m_DebugName = name; }

	protected:
		std::string m_DebugName;

	};

}