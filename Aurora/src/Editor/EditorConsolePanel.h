#pragma once

#include "EditorPanel.h"
#include "EditorConsoleMessage.h"
#include "Core/Base.h"

#include <imgui/imgui.h>

namespace Aurora {

	class EditorConsolePanel : public EditorPanel
	{
	public:
		EditorConsolePanel();
		~EditorConsolePanel();

		static Ref<EditorConsolePanel> Create();

		virtual void OnImGuiRender(bool& isOpen) override;
		virtual void OnEvent(Event& e) override;

	private:
		void RenderMenu(const ImVec2& size);
		void RenderConsole(const ImVec2& size);
		const ImVec4& GetMessageColor(const ConsoleMessage& msg) const;
		const char* GetMessageType(const ConsoleMessage& message) const;

	private:
		static void PushMessage(const ConsoleMessage& msg);

	private:
		std::vector<ConsoleMessage> m_MessageBuffer;
		int16_t m_MessageLevelFilters = (int16_t)ConsoleMessageFlags::All;

		bool m_EnableScrollToLatest = true;
		bool m_ScrollToLatest = false;
		bool m_DetailedPanelOpen = false;
		float m_PreviousScrollY = 0.0f;

	private:
		static EditorConsolePanel* s_Instance;

		friend class EditorConsoleSink;

	};

}