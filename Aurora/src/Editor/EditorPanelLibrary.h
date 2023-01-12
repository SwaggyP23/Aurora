#pragma once

#include "EditorPanel.h"
#include "Core/Base.h"
#include "Core/Hash.h"

#include <array>
#include <unordered_map>

namespace Aurora {

	// This will be serialized into a file with extension .aeditor so that the engine can know on startup the state it was left in last start
	struct PanelSpecification
	{
		const char* ID = "";
		const char* Name = "";
		Ref<EditorPanel> Panel = nullptr;
		bool IsOpen = false;
	};

	// Used to sort the panels under different categories
	enum class PanelCategory
	{
		View = 0,
		Edit = 1,
		EnumSize
	};

	class EditorPanelsLibrary
	{
	public:
		EditorPanelsLibrary() = default;
		~EditorPanelsLibrary();

		static Scope<EditorPanelsLibrary> Create();

		void OnImGuiRender();
		void OnEvent(Event& e);
		void OnProjectChanged(Ref<Project> project);

		void SetSceneContext(Ref<Scene> scene);

		void Serialize();
		void Deserialize();

		void RemovePanel(const char* strId);

		PanelSpecification* GetPanelSpec(uint32_t id);
		const PanelSpecification* GetPanelSpec(uint32_t id) const;

		std::unordered_map<uint32_t, PanelSpecification>& GetPanels(PanelCategory category) { return m_Panels[(size_t)category]; }
		const std::unordered_map<uint32_t, PanelSpecification>& GetPanels(PanelCategory category) const { return m_Panels[(size_t)category]; }

		template<typename TPanel>
		Ref<TPanel> AddPanel(const PanelSpecification& spec, PanelCategory category)
		{
			static_assert(std::is_base_of<EditorPanel, TPanel>::value, "The panel to be added should derive from EditorPanel");

			std::unordered_map<uint32_t, PanelSpecification>& panelMap = m_Panels[(size_t)category];

			uint32_t panelhash = Hash::GenerateFNVHash(spec.ID);
			if (panelMap.find(panelhash) != panelMap.end())
			{
				AR_CORE_ERROR_TAG("EditorPanelsLibrary", "Trying to add panel that has already been added!");
				return nullptr;
			}

			panelMap[panelhash] = spec;
			return panelMap[panelhash].Panel;
		}

		template<typename TPanel, typename... Args>
		Ref<TPanel> AddPanel(PanelCategory category, const char* strId, bool isOpenByDefault, Args&&... args)
		{
			return AddPanel<TPanel>({ strId, strId, TPanel::Create(std::forward<Args>(args)...), isOpenByDefault }, category);
		}

		template<typename TPanel, typename... Args>
		Ref<TPanel> AddPanel(PanelCategory category, const char* strId, const char* displayName, bool isOpenByDefault, Args&&... args)
		{
			return AddPanel<TPanel>({ strId, displayName, TPanel::Create(std::forward<Args>(args)...), isOpenByDefault }, category);
		}

		template<typename TPanel>
		Ref<TPanel> GetPanel(const char* strID)
		{
			static_assert(std::is_base_of<EditorPanel, TPanel>::value, "Trying to retrieve a panel that is not derived from EditorPanel!");

			uint32_t panelhash = Hash::GenerateFNVHash(strID);

			for (const auto& panelMap : m_Panels)
			{
				if (panelMap.find(panelhash) != panelMap.end())
					return panelMap.at(panelhash).Panel.As<TPanel>();
			}

			AR_CORE_ERROR_TAG("EditorPanelsLibrary", "Did not find panel with id {0}", strID);
			return nullptr;	
		}

	private:
		std::array<std::unordered_map<uint32_t, PanelSpecification>, (size_t)PanelCategory::EnumSize> m_Panels;

	};

}