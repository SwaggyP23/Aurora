#pragma once

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

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnEvent(Event& e) override;

		void Begin();
		void End();

		void SetBlockEvents(bool state) { m_BlockEvents = state; }

		void SetDarkThemeColor();

	private:
		bool m_BlockEvents = true;

	};

}