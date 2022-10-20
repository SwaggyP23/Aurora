#pragma once

#include "EditorPanel.h"
#include "Core/Base.h"

namespace Aurora {

	class EditorConsolePanel : public EditorPanel
	{
	public:
		EditorConsolePanel() = default;
		~EditorConsolePanel() = default;

		static Ref<EditorConsolePanel> Create();

		void OnImGuiRender(bool& isOpen) override {}

	private:

		bool m_EnableScrollToLatest = true;
		bool m_ScrollToLatest = false;
		bool m_ClearOnPlay = false;

	};

}