#pragma once

#include "Core/UUID.h"
#include "Scene/Entity.h"

#include <unordered_map>

namespace Aurora {

	enum class SelectionContext
	{
		Global = 0,
		Scene,
		ContentBrowser
	};

	// Since we have an EditorPanelsLibrary we cant handle the scene hierarchy selection context directly interacting with the scene hierarchy panel
	// So we need to manage the selection context in some type of global state/map which is what this is
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