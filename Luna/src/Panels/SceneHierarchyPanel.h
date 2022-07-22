#pragma once

#include "Core/Base.h"
#include "Scene/Scene.h"
#include "Scene/Entity.h"

namespace Aurora {

	class SceneHierarchyPanel
	{
	public:
		SceneHierarchyPanel() = default;
		SceneHierarchyPanel(const Ref<Scene>& scene);

		void SetContext(const Ref<Scene>& context); // The context for this panel is the scene since it displays the scene's contents

		void OnImGuiRender();

	private:
		void DrawEntityNode(Entity entity);
		void DrawComponents(Entity entity);

	private:
		Ref<Scene> m_Context;
		Entity m_SelectionContext;

	};

}