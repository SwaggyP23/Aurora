#pragma once

#include <string>
#include "Core/TimeStep.h"
#include "Events/Events.h"

class Layer // This is an interface
{
public:
	Layer(const std::string& name = "Default layer");
	virtual ~Layer();

	virtual void onAttach() {}
	virtual void onDetach() {}
	virtual void onUpdate(TimeStep ts) {}
	virtual void onImGuiRender() {}
	virtual void onEvent(Event& e) {}

	inline const std::string& getName() const { return m_Name; }

private:
	std::string m_Name;
};