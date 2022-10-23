#pragma once

#include "Editor/EditorPanel.h"
#include "Core/Base.h"

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

namespace Aurora {

	enum class CBItemType
	{
		None = 0,
		Directory,
		Asset
	};

	struct ContextBrowserItem
	{
		std::string Name;
		CBItemType Type;
	};

	class ContentBrowserPanel : public EditorPanel
	{
	public:
		ContentBrowserPanel() = default;
		virtual ~ContentBrowserPanel() = default;

		static Ref<ContentBrowserPanel> Create();

		virtual void OnImGuiRender(bool& isOpen) override {}
		virtual void OnEvent(Event& e) override {}

		//ItemType GetItemType();

	private:
		void Refresh() {}
		void GoUpADirectory() {}
		void GoInADirectory() {}

	private:
		std::vector<ContextBrowserItem>  m_Items;

		bool m_IsPanelHovered = false;
		bool m_IsPanelFocused = false;
		bool m_IsAnyItemHovered = false;

	};

}