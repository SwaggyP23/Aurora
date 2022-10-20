#pragma once

#include "EditorPanel.h"
#include "EditorSelectionManager.h"
#include "Core/Base.h"
#include "Core/Input/Input.h"
#include "Core/Events/KeyEvents.h"
#include "Scene/Entity.h"
#include "Scene/Scene.h"

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

namespace Aurora {

	class SceneHierarchyPanel : public EditorPanel
	{
	public:
		SceneHierarchyPanel() = default;
		SceneHierarchyPanel(const Ref<Scene>& scene, SelectionContext context);
		~SceneHierarchyPanel() = default;

		static Ref<SceneHierarchyPanel> Create();
		static Ref<SceneHierarchyPanel> Create(const Ref<Scene>& scene, SelectionContext context);

		void SetEntityDeletedCallback(const std::function<void(Entity)>& callback) { m_EntityDeletedCallback = callback; }

		virtual void SetSceneContext(const Ref<Scene>& scene) override;
		Ref<Scene> GetSceneContext() const { return m_Context; }

		virtual void OnImGuiRender(bool& isOpen) override;
		virtual void OnEvent(Event& e) override;

		static SelectionContext GetActiveSelectionContext();

	private:
		bool OnKeyPressedEvent(KeyPressedEvent& e);
		void DrawEntityCreatePopupMenu(Entity entity);

		void DrawEntityNode(Entity entity, const std::string& searchedString = {});
		void DrawComponents(const std::vector<UUID>& entityIDs);
		void DrawVec3Control(const std::string& label, glm::vec3& values, float resetValue = 0.0f, float colomnWidth = 100.0f, float min = 0.0f, float max = 0.0f, float stepValue = 0.1f);

		void OnExternalEntityDestroyed(Entity entity);

	private:
		Ref<Scene> m_Context;

		ImRect m_SceneHierarchyTableRect;
		std::vector<Entity> m_SortedEntities;

		std::function<void(Entity)> m_EntityDeletedCallback;

		bool m_IsPanelFocused = false;
		bool m_IsHierarchyOrPropertiesFocused = false;
		bool m_ShowSceneHierarchyPanel = true;
		bool m_ShowPropertiesPanel = true;

	};

}