#pragma once

#include "EditorPanel.h"
#include "Core/Events/KeyEvents.h"

namespace Aurora {

	class ShadersPanel : public EditorPanel
	{
	public:
		ShadersPanel() = default;
		~ShadersPanel() = default;

		static Ref<ShadersPanel> Create();

		// TODO:
		virtual void OnImGuiRender(bool& isOpen) override;
		virtual void OnEvent(Event& e) override;

	private:
		bool OnKeyPressed(KeyPressedEvent& e);

	};

}