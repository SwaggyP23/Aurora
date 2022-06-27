#pragma once

/*
 * The ImGuiLayer and Layers and Events system is all thanks to The Cherno which infact this whole thing 
 * is in thanks to him since he is the main reference and then there is also the LearnOpenGL Book
 */

#include "Layers/Layer.h"
#include "Events/ApplicationEvents.h"
#include "Events/MouseEvents.h"
#include "Events/KeyEvents.h"
#include <glm/glm.hpp>

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
	inline glm::vec4 getClearColor() const { return m_Color; }

private:
	float m_Time = 0.0f;
	float m_Blend = 0.0f;
	glm::vec4 m_Color = glm::vec4(0.2f, 0.7f, 0.8f, 1.0f); // Initial clear color.

};