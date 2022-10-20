#pragma once

#include "Editor/EditorPanel.h"
#include "Core/Base.h"

namespace Aurora {

	class ContentBrowserPanel : public EditorPanel
	{
	public:
		ContentBrowserPanel() = default;
		~ContentBrowserPanel() = default;

		static Ref<ContentBrowserPanel> Create();

		void OnImGuiRender(bool& isOpen) override {}

	private:
		
		bool m_IsPanelHovered = false;
		bool m_IsPanelFocused = false;
		bool m_IsAnyItemHovered = false;

	};

}