#pragma once

#include "Layers/Layer.h"
#include "Events/ApplicationEvents.h"
#include "Events/MouseEvents.h"
#include "Events/KeyEvents.h"

class ImGuiLayer : public Layer
{
public:
	ImGuiLayer();
	~ImGuiLayer();

	void onAttach();
	void onDetach();
	void onUpdate();
	void onEvent(Event& e);

	float getBlend() const { return blend; }

private:
	bool onMouseMovedEvent(MouseMovedEvent& e);
	bool onMouseScrolledEvent(MouseScrolledEvent& e);
	bool onMouseButtonPressedEvent(MouseButtonPressedEvent& e);
	bool onMouseButtonReleasedEvent(MouseButtonReleasedEvent& e);
	bool onKeyPressedEvent(KeyPressedEvent& e);
	bool onKeyReleasedEvent(KeyReleasedEvent& e);
	bool onKeyTypedEvent(KeyTypedEvent& e);
	bool onWindowResizeEvent(WindowResizeEvent& e);

private:
	float blend = 0.0f;
	float m_Time = 0.0f;

};