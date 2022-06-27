#pragma once

/*
 * The ImGuiLayer and Layers and Events system is all thanks to The Cherno which infact this whole thing 
 * is in thanks to him since he is the main reference and then there is also the LearnOpenGL Book
 */

#include "Layers/Layer.h"
#include "Events/ApplicationEvents.h"
#include "Events/MouseEvents.h"
#include "Events/KeyEvents.h"

class ImGuiLayer : public Layer
{
public:
	ImGuiLayer();
	~ImGuiLayer();

	virtual void onAttach() override;
	virtual void onDetach() override;
	virtual void onImGuiRender() override;

	void begin();
	void end();

	inline float getBlend() const { return m_Blend; }
	inline void setBlend(float blend) { m_Blend = blend; }

private:
	float m_Time = 0.0f;
	float m_Blend = 0.0f;

};