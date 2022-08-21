#pragma once

#include "Layers/Layer.h"
#include "Events/ApplicationEvents.h"
#include "Events/MouseEvents.h"
#include "Events/KeyEvents.h"
#include "FontsLibrary.h"

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
		void LoadAndAddFonts();

	private:
		bool m_BlockEvents = true;
		FontsLibrary m_FontsLibrary;

		friend class EditorLayer; // The editor is build on ImGui so makes sense to be related to the ImGui layer

	};

}