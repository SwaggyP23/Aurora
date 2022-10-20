#include "Aurorapch.h"
#include "EditorSelectionManager.h"

#include "Core/Application.h"

namespace Aurora {

	void SelectionManager::Select(SelectionContext context, UUID selectionID)
	{
		std::vector<UUID>& selections = s_Selections[context];
		if (std::find(selections.begin(), selections.end(), selectionID) != selections.end())
			return;

		selections.push_back(selectionID);
	}

	bool SelectionManager::IsSelected(UUID selectionID)
	{
		for (auto& [context, selections] : s_Selections)
		{
			for (const UUID& id : selections)
			{
				if (id == selectionID)
					return true;
			}
		}

		return false;
	}

	bool SelectionManager::IsSelected(SelectionContext context, UUID selectionID)
	{
		const std::vector<UUID>& selections = s_Selections[context];
		return std::find(selections.begin(), selections.end(), selectionID) != selections.end();
	}

	// TODO: Once parenting entities is a thing
	//bool SelectionManager::IsEntityOrAncestorSelected(const Entity entity)
	//{
	//	return false;
	//}

	//bool SelectionManager::IsEntityOrAncestorSelected(SelectionContext context, const Entity entity)
	//{
	//	return false;
	//}

	void SelectionManager::Deselect(UUID selectionID)
	{
		for (auto& [context, selections] : s_Selections)
		{
			auto itr = std::find(selections.begin(), selections.end(), selectionID);
			if (itr == selections.end())
				continue;

			// TODO:
			//Application::GetApp().DispatchEvent<SelectionChangedEvent>(context, selectionID, false);
			selections.erase(itr);
			break;
		}
	}

	void SelectionManager::Deselect(SelectionContext context, UUID selectionID)
	{
		std::vector<UUID>& selections = s_Selections[context];
		auto itr = std::find(selections.begin(), selections.end(), selectionID);
		if (itr == selections.end())
			return;

		selections.erase(itr);
	}

	void SelectionManager::DeselectAll()
	{
		for (auto& [context, selections] : s_Selections)
		{
			for (const UUID& selectionID : selections)
			{
				// TODO:
				//Application::GetApp().DispatchEvent<SelectionChangedEvent>(context, selectionID, false);
			}

			selections.clear();
		}
	}

	void SelectionManager::DeselectAll(SelectionContext context)
	{
		std::vector<UUID>& selections = s_Selections[context];
		for (const auto& selectionID : selections)
		{
			// TODO:
			//Application::GetApp().DispatchEvent<SelectionChangedEvent>(context, selectionID, false);
		}

		selections.clear();
	}

	UUID SelectionManager::GetSelection(SelectionContext context, size_t index)
	{
		AR_CORE_ASSERT(index >= 0 && index < s_Selections[context].size());
		return s_Selections[context][index];
	}

	size_t SelectionManager::GetSelectionCount(SelectionContext context)
	{
		return s_Selections[context].size();
	}

	std::unordered_map<SelectionContext, std::vector<UUID>> SelectionManager::s_Selections;

}