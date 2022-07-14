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

namespace Aurora {

	class ImGuiLayer : public Layer
	{
	public:
		ImGuiLayer();
		~ImGuiLayer();

		virtual void onAttach() override;
		virtual void onDetach() override;
		virtual void onEvent(Event& e) override;

		void begin();
		void end();

		void SetBlockEvents(bool state) { m_BlockEvents = state; }

	private:
		bool m_BlockEvents = true;

	};

}