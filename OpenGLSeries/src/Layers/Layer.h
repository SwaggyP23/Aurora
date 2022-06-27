#pragma once

#include <string>
#include "Events/Events.h"

class Layer
{
public:
	Layer(const std::string& name = "Default layer");
	virtual ~Layer();

	virtual void onAttach() {}
	virtual void onDetach() {}
	virtual void onUpdate() {}
	virtual void onImGuiRender() {}
	virtual void onEvent(Event& e) {}

	inline const std::string& getName() const { return m_Name; }

private:
	std::string m_Name;
};