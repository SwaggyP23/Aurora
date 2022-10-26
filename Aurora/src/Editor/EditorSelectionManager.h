#pragma once

#include "Core/UUID.h"
#include "Scene/Entity.h"

#include <unordered_map>

/*
 * Why does this exist? Since multiple editor panels and multiple engine systems rely on the currently selected entity, We cant just store the
 * selection context inside the SceneHierarchyPanel since that will be a pain to always retrieve it and if it has children. So what the
 * SelectionManager is just a Global entity State/Map of selected entities and it helps with providing a global way of getting the selected entity/ies
 */

namespace Aurora {

	enum class SelectionContext
	{
		Global = 0,
		Scene,
		ContentBrowser
	};

	class SelectionManager
	{
	public:
		static void Select(SelectionContext context, UUID selectionID);
		static bool IsSelected(UUID selectionID);
		static bool IsSelected(SelectionContext context, UUID selectionID);
		// TODO: Once parenting entities is a thing
		//static bool IsEntityOrAncestorSelected(const Entity entity);
		//static bool IsEntityOrAncestorSelected(SelectionContext context, const Entity entity);
		static void Deselect(UUID selectionID);
		static void Deselect(SelectionContext context, UUID selectionID);
		static void DeselectAll();
		static void DeselectAll(SelectionContext context);
		static UUID GetSelection(SelectionContext context, size_t index);

		static size_t GetSelectionCount(SelectionContext context);
		inline static const std::vector<UUID>& GetSelections(SelectionContext context) { return s_Selections[context]; }

	private:
		// Stores a vector since i am preparing for entity parenting
		static std::unordered_map<SelectionContext, std::vector<UUID>> s_Selections;

	};

}