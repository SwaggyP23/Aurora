#pragma once

#include "Core/Layer.h"
#include "Core/Events/ApplicationEvents.h"
#include "Core/Events/MouseEvents.h"
#include "Core/Events/KeyEvents.h"
#include "FontsLibrary.h"

namespace Aurora {

	class ImGuiLayer : public Layer
	{
	public:
		ImGuiLayer();
		virtual ~ImGuiLayer();

		virtual void OnAttach() override;
		virtual void OnDetach() override;

		void Begin();
		void End();

		void SetDarkThemeColor();

	private:
		void LoadAndAddFonts();

	private:
		FontsLibrary m_FontsLibrary;

		friend class EditorLayer; // The editor is build on ImGui so makes sense to be related to the ImGui layer

	};

}