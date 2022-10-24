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

	// TODO: Should it be refcounted??
	class ContentBrowserItem : public RefCountedObject
	{
	public:
		ContentBrowserItem(const Ref<Texture2D>& texture) {}
		virtual ~ContentBrowserItem() = default;

		CBItemType GetItemType() const { return m_Type; }
		const std::string& GetItemName() const { return m_Name; }

	private:
		std::string m_Name;
		CBItemType m_Type;

		friend class ContentBrowserPanel;
	};

	class ContentBrowserPanel : public EditorPanel
	{
	public:
		ContentBrowserPanel() = default;
		virtual ~ContentBrowserPanel() = default;

		static Ref<ContentBrowserPanel> Create();

		virtual void OnImGuiRender(bool& isOpen) override {}
		virtual void OnEvent(Event& e) override {}

	private:
		void Refresh() {}
		void GoUpADirectory() {}
		void GoInADirectory() {}

	private:
		std::vector<ContentBrowserItem> m_Items;

		bool m_IsPanelHovered = false;
		bool m_IsPanelFocused = false;
		bool m_IsAnyItemHovered = false;

	};

}