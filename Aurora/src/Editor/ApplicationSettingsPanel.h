#pragma once

#include "EditorPanel.h"
#include "Renderer/SceneRenderer.h"

namespace YAML {

	class Emitter;
	class Node;

}

namespace Aurora {

	class ApplicationSettingsPanel : public EditorPanel
	{
	public:
		ApplicationSettingsPanel(SceneRenderer* viewportRenderer);
		virtual ~ApplicationSettingsPanel() = default;

		static Ref<ApplicationSettingsPanel> Create(SceneRenderer* viewportRenderer);

		virtual void OnImGuiRender(bool& isOpen) override;

		bool ShowGizmos() const;
		bool ShowBoundingBoxes() const;
		bool ShowIcons() const;
		bool ShowGrid() const;
		bool AllowGizmoFlip() const;

	private:
		static void Serialize(YAML::Emitter& mainEmitter);
		static void Deserialize(const YAML::Node& mainNode);

		friend class EditorPanelsLibrary;

	};

}