#pragma once

#include "EditorPanel.h"

namespace YAML {

	class Emitter;
	class Node;

}

namespace Aurora {

	class EditorStylePanel : public EditorPanel
	{
	public:
		EditorStylePanel() = default;
		virtual ~EditorStylePanel() = default;

		static Ref<EditorStylePanel> Create();

		virtual void OnImGuiRender(bool& isOpen) override;

	private:
		static void Serialize(YAML::Emitter& mainEmitter);
		static void Deserialize(const YAML::Node& mainNode);
		void ShowFontPickerUI();

		friend class EditorPanelsLibrary;

	};

}