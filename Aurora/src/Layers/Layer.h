#pragma once

#include <string>
#include "Core/TimeStep.h"
#include "Events/Events.h"

namespace Aurora {

	class Layer // This is an interface
	{
	public:
		Layer(const std::string& name = "Default layer");
		virtual ~Layer();

		virtual void OnAttach() {}
		virtual void OnDetach() {}
		virtual void OnUpdate(TimeStep ts) {}
		virtual void OnImGuiRender() {}
		virtual void OnEvent(Event& e) {}

		inline const std::string& getName() const { return m_Name; }

	private:
		std::string m_Name;
	};

}