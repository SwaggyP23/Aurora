#include "Aurorapch.h"
#include "EditorConsolePanel.h"

#include "Core/Random.h"
#include "ImGui/FontAwesome.h"
#include "ImGui/ImGuiUtils.h"

namespace Aurora {

	EditorConsolePanel* EditorConsolePanel::s_Instance = nullptr;

	// TODO: Maybe change to green???
	static const ImVec4 s_InfoTint = ImVec4(0.0f, 0.431372549f, 1.0f, 1.0f);
	static const ImVec4 s_WarningTint = ImVec4(1.0f, 0.890196078f, 0.0588235294f, 1.0f);
	static const ImVec4 s_ErrorTint = ImVec4(1.0f, 0.309803922f, 0.309803922f, 1.0f);

	Ref<EditorConsolePanel> EditorConsolePanel::Create()
	{
		return CreateRef<EditorConsolePanel>();
	}

	EditorConsolePanel::EditorConsolePanel()
	{
		AR_CORE_ASSERT(s_Instance == nullptr);
		s_Instance = this;

		m_MessageBuffer.reserve(500);
	}

	EditorConsolePanel::~EditorConsolePanel()
	{
		s_Instance = nullptr;
	}

	void EditorConsolePanel::OnImGuiRender(bool& isOpen)
	{
		if (ImGui::Begin("Console", &isOpen))
		{
			ImVec2 consoleSize = ImGui::GetContentRegionAvail();
			consoleSize.y -= 32.0f;

			RenderMenu({ consoleSize.x, 28.0f });
			RenderConsole(consoleSize);
		}

		ImGui::End();
	}

	void EditorConsolePanel::OnEvent(Event& e)
	{
		// TODO: Clearing OnScenePlay??
	}

	void EditorConsolePanel::RenderMenu(const ImVec2& size)
	{
		ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0.0f, 0.0f });

		ImGui::BeginChild("Toolbar", size);

		if (ImGui::Button("Clear", ImVec2{ 75.0f, 28.0f }))
			m_MessageBuffer.clear();

		{
			const ImGuiStyle& style = ImGui::GetStyle();
			const ImVec2 buttonSize = ImVec2{ 28.0f, 28.0f };

			ImGui::SameLine(ImGui::GetContentRegionAvail().x - 110.0f, 0.0f);
			ImVec4 textColor = (m_MessageLevelFilters & (int16_t)ConsoleMessageFlags::Info) ? s_InfoTint : style.Colors[ImGuiCol_TextDisabled];
			ImGui::PushStyleColor(ImGuiCol_Text, textColor);
			if(ImGui::Button(AR_ICON_INFO_CIRCLE, buttonSize))
				m_MessageLevelFilters ^= (int16_t)ConsoleMessageFlags::Info;
			ImGui::PopStyleColor();

			ImGui::SameLine();
			textColor = (m_MessageLevelFilters & (int16_t)ConsoleMessageFlags::Warning) ? s_WarningTint : style.Colors[ImGuiCol_TextDisabled];
			ImGui::PushStyleColor(ImGuiCol_Text, textColor);
			if(ImGui::Button(AR_ICON_EXCLAMATION_TRIANGLE, buttonSize))
				m_MessageLevelFilters ^= (int16_t)ConsoleMessageFlags::Warning;
			ImGui::PopStyleColor();

			ImGui::SameLine();
			textColor = (m_MessageLevelFilters & (int16_t)ConsoleMessageFlags::Error) ? s_ErrorTint : style.Colors[ImGuiCol_TextDisabled];
			ImGui::PushStyleColor(ImGuiCol_Text, textColor);
			if (ImGui::Button(AR_ICON_EXCLAMATION_CIRCLE, buttonSize))
				m_MessageLevelFilters ^= (int16_t)ConsoleMessageFlags::Error;
			ImGui::PopStyleColor();
		}

		ImGui::EndChild();

		ImGui::PopStyleVar(2);
	}

	void EditorConsolePanel::RenderConsole(const ImVec2& size)
	{
		static const char* columns[] = { "Type", "Timestamp", "Message" };

		ImGuiUtils::Table("Log", columns, 3, size, [&]()
		{
			// Do some checks...
			float scrollY = ImGui::GetScrollY();
			if (scrollY < m_PreviousScrollY)
				m_EnableScrollToLatest = false;

			if (scrollY > ImGui::GetScrollMaxY())
				m_EnableScrollToLatest = true;

			// Update scroll y
			m_PreviousScrollY = scrollY;

			constexpr float rowHeight = 24.0f;
			for (uint32_t i = 0; i < m_MessageBuffer.size(); i++)
			{
				const ConsoleMessage& msg = m_MessageBuffer[i];

				if (!(m_MessageLevelFilters & (int16_t)msg.Flags))
					continue;

				ImGui::PushID(&msg);

				bool clicked = ImGuiUtils::TableRowClickable(msg.ShortMessage.c_str(), rowHeight);

				ImGuiUtils::Separator(ImVec2{ 4.0f, rowHeight }, GetMessageColor(msg));
				ImGui::SameLine();
				ImGui::Text(GetMessageType(msg));
				ImGui::TableNextColumn();
				ImGuiUtils::ShiftCursorX(4.0f);

				std::stringstream timeString;
				tm timeBuffer;
				localtime_s(&timeBuffer, &msg.Time);
				timeString << std::put_time(&timeBuffer, "%T");
				ImGui::Text(timeString.str().c_str());

				ImGui::TableNextColumn();
				ImGuiUtils::ShiftCursorX(4.0f);
				ImGui::Text(msg.ShortMessage.c_str());

				if (i == m_MessageBuffer.size() - 1 && m_ScrollToLatest)
				{
					ImGui::ScrollToItem();
					m_ScrollToLatest = false;
				}

				if (clicked)
				{
					ImGui::OpenPopup("Detailed Message");
					ImVec2 size = ImGui::GetMainViewport()->Size;
					ImGui::SetNextWindowSize({ size.x * 0.5f, size.y * 0.5f });
					ImGui::SetNextWindowPos({ size.x * 0.5f, size.y * 0.5f }, 0, { 0.5, 0.5 });
					m_DetailedPanelOpen = true;
				}

				if (m_DetailedPanelOpen)
				{
					ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4.0f, 4.0f));
					ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4.0f, 8.0f));

					if (ImGui::BeginPopupModal("Detailed Message", &m_DetailedPanelOpen, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize))
					{
						ImGuiContext& g = *GImGui;
						float size = g.Font->FontSize;
						g.Font->FontSize /= 1.2f;
						ImGui::TextWrapped(msg.LongMessage.c_str());
						g.Font->FontSize = size;
						ImGui::EndPopup();
					}

					ImGui::PopStyleVar(2);
				}

				ImGui::PopID();
			}
		});
	}

	const ImVec4& EditorConsolePanel::GetMessageColor(const ConsoleMessage& msg) const
	{
		if (msg.Flags & (int16_t)ConsoleMessageFlags::Warning) return s_WarningTint;
		if (msg.Flags & (int16_t)ConsoleMessageFlags::Error) return s_ErrorTint;
		return s_InfoTint;
	}

	void EditorConsolePanel::PushMessage(const ConsoleMessage& msg)
	{
		if (s_Instance == nullptr)
			return;

		s_Instance->m_MessageBuffer.push_back(msg);

		if (s_Instance->m_EnableScrollToLatest)
			s_Instance->m_ScrollToLatest = true;
	}

	const char* EditorConsolePanel::GetMessageType(const ConsoleMessage& message) const
	{
		if (message.Flags & (int16_t)ConsoleMessageFlags::Info) return "Info";
		if (message.Flags & (int16_t)ConsoleMessageFlags::Warning) return "Warning";
		if (message.Flags & (int16_t)ConsoleMessageFlags::Error) return "Error";
		return "Unknown";
	}

}