#include "Aurorapch.h"
#include "AuroraTODOPanel.h"

#include "ImGui/ImGuiUtils.h"

#include <yaml-cpp/yaml.h>

namespace Aurora {

	static const ImVec4 s_UrgentNoteColor = { 1.0f, 0.309803922f, 0.309803922f, 1.0f };
	static const ImVec4 s_WhenPossibleNoteColor = { 0.0f, 0.431372549f, 1.0f, 1.0f };

	namespace Utils {

		TODOPanel::Importance ImportanceLevelFromString(const std::string& level)
		{
			if (level == "Urgent") return TODOPanel::Importance::Urgent;
			if (level == "WhenPossible") return TODOPanel::Importance::WhenPossible;

			AR_CORE_ASSERT(false, "Unknown level!");
			return TODOPanel::Importance::None;
		}

		std::string ImportanceLevelToString(TODOPanel::Importance level)
		{
			switch (level)
			{
				case TODOPanel::Importance::Urgent:			return "Urgent";
				case TODOPanel::Importance::WhenPossible:	return "WhenPossible";
			}

			AR_CORE_ASSERT(false, "Unknown level!");
			return "None";
		}

	}

	Ref<TODOPanel> TODOPanel::Create()
	{
		return CreateRef<TODOPanel>();
	}

	TODOPanel::TODOPanel()
	{
		Deserialize();
	}

	TODOPanel::~TODOPanel()
	{
		Serialize();
	}

	void TODOPanel::OnImGuiRender(bool& isOpen)
	{
		if (ImGui::Begin("TODO List", &isOpen))
		{
			static int indexToOpenDetailed = -1;
			constexpr float buttonWidth = 100.0f;
			constexpr float rowHeight = 24.0f;
			constexpr const char* hint = "Enter something to do...";

			const ImVec2 buttonSize = ImVec2{ 28.0f, 28.0f };
			ImGuiStyle& style = ImGui::GetStyle();

			ImGui::Columns(2);
			ImGui::SetColumnWidth(0, 150.0f);

			ImGuiUtils::Separator(ImVec2{ 4.0f, rowHeight }, ImVec4{ 0.345f, 1.0f, 0.356f, 1.0f });
			ImGui::SameLine();
			ImGui::Text("Importance");
			ImGui::NextColumn();

			ImGui::Text("TODO");

			ImGui::SameLine();
			ImGuiUtils::ShiftCursorX(ImGui::GetContentRegionAvail().x - 55.0f);

			ImVec4 textColor = (m_NoteFilters & (int16_t)Importance::Urgent) ? s_UrgentNoteColor : style.Colors[ImGuiCol_TextDisabled];
			ImGui::PushStyleColor(ImGuiCol_Text, textColor);
			if (ImGui::Button(AR_ICON_EXCLAMATION_CIRCLE, buttonSize))
			{
				m_NoteFilters ^= (int16_t)Importance::Urgent;
			}
			ImGui::PopStyleColor();

			ImGui::SameLine();
			textColor = (m_NoteFilters & (int16_t)Importance::WhenPossible) ? s_WhenPossibleNoteColor : style.Colors[ImGuiCol_TextDisabled];
			ImGui::PushStyleColor(ImGuiCol_Text, textColor);
			if (ImGui::Button(AR_ICON_INFO_CIRCLE, buttonSize))
			{
				m_NoteFilters ^= (int16_t)Importance::WhenPossible;
			}
			ImGui::PopStyleColor();

			ImGui::NextColumn();

			std::sort(m_Notes.begin(), m_Notes.end(), [](const Note& note1, const Note& note2)
			{
				return note1.ImportanceLevel < note2.ImportanceLevel;
			});

			int indexToRemoveAt = -1;
			for (size_t i = 0; i < m_Notes.size(); i++)
			{
				if (!(m_NoteFilters & (int16_t)m_Notes[i].ImportanceLevel))
					continue;

				ImGui::PushID((int)i);
				ImGuiUtils::Separator(ImVec2{ 4.0f, rowHeight }, GetNoteColor(m_Notes[i]));

				ImGui::SameLine();
				ImGui::Text(Utils::ImportanceLevelToString(m_Notes[i].ImportanceLevel).c_str());

				ImGui::NextColumn();

				std::string* value = &m_Notes[i].Description;

				ImGui::PushItemWidth(-1);
				ImGui::InputTextWithHint(ImGuiUtils::GenerateID(), hint, value, ImGuiInputTextFlags_AlwaysInsertMode);
				ImGui::PopItemWidth();

				if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
				{
					m_DetailedPanelOpen = true;
					indexToOpenDetailed = (int)i;
				}
				else if(ImGui::IsItemClicked(ImGuiMouseButton_Right))
				{
					ImGui::OpenPopup("TODOPanelPopUp");
				}

				if (ImGui::BeginPopup("TODOPanelPopUp"))
				{
					if (ImGui::MenuItem("Delete"))
					{
						indexToRemoveAt = (int)i;
					}

					ImGui::EndPopup();
				}

				ImGui::NextColumn();

				ImGui::PopID();
			}

			if (indexToRemoveAt != -1)
			{
				RemoveNote(indexToRemoveAt);
			}

			ImGui::Columns(1);

			ImGuiUtils::ShiftCursorX(ImGui::GetContentRegionAvail().x / 2.0f - buttonWidth / 2.0f);

			if (ImGui::Button("Add Note", ImVec2{ buttonWidth, rowHeight }))
			{
				m_TypePanelOpen = true;
			}

			if (m_TypePanelOpen)
			{
				ImGui::OpenPopup("Choose Note Type");

				ImVec2 center = ImGui::GetMainViewport()->GetCenter();
				ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

				if (ImGui::BeginPopupModal("Choose Note Type", &m_TypePanelOpen, ImGuiWindowFlags_AlwaysAutoResize))
				{
					if (ImGui::Button("Urgent!"))
					{
						AddNote({ Importance::Urgent, "" });

						m_TypePanelOpen = false;
						ImGui::CloseCurrentPopup();
					}

					ImGui::SameLine();

					if (ImGui::Button("When Possible!"))
					{
						AddNote({ Importance::WhenPossible, "" });

						m_TypePanelOpen = false;
						ImGui::CloseCurrentPopup();
					}

					ImGui::EndPopup();
				}
			}

			if (m_DetailedPanelOpen)
			{
				ImGui::OpenPopup("DetailedPanel");

				ImVec2 center = ImGui::GetMainViewport()->GetCenter();
				ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2{ 0.5f, 0.5f });

				if (ImGui::BeginPopupModal("DetailedPanel", &m_DetailedPanelOpen))
				{
					ImGui::InputTextMultiline(ImGuiUtils::GenerateID(), &m_Notes[indexToOpenDetailed].Description, ImGui::GetContentRegionAvail());

					ImGui::EndPopup();
				}
			}
		}

		ImGui::End();
	}

	void TODOPanel::OnEvent(Event& e)
	{
	}

	void TODOPanel::AddNote(const Note& note)
	{
		m_Notes.push_back(note);
	}

	void TODOPanel::RemoveNote(int indexToRemoveAt)
	{
		m_Notes.erase(m_Notes.begin() + indexToRemoveAt);
	}

	void TODOPanel::Serialize()
	{
		YAML::Emitter out;

		out << YAML::BeginMap;

		out << YAML::Key << "TODOs" << YAML::BeginSeq;
		for (size_t i = 0; i < m_Notes.size(); i++)
		{
			const Note& note = m_Notes[i];

			out << YAML::BeginMap;

			out << YAML::Key << "Importance" << YAML::Value << Utils::ImportanceLevelToString(note.ImportanceLevel);
			out << YAML::Key << "TODO" << YAML::Key << note.Description;

			out << YAML::EndMap;
		}
		out << YAML::EndSeq;

		out << YAML::EndMap;

		std::ofstream ofStream("Config/TODOPanel.apanel");
		ofStream << out.c_str();
		ofStream.close();
	}

	void TODOPanel::Deserialize()
	{
		std::filesystem::path path = "Config/TODOPanel.apanel";
		if (!std::filesystem::exists(path))
			return;

		std::ifstream ifStream(path);
		AR_CORE_CHECK(ifStream);
		std::stringstream strStream;

		strStream << ifStream.rdbuf();

		YAML::Node data = YAML::Load(strStream.str());
		if (!data["TODOs"])
		{
			AR_CONSOLE_LOG_ERROR("[TODOPanel] Failed to load the TODOs that were saved if any!");
			return;
		}

		m_Notes.reserve(data.size());
		for (auto todoNode : data["TODOs"])
		{
			Note note = {};
			note.ImportanceLevel = Utils::ImportanceLevelFromString(todoNode["Importance"].as<std::string>());
			note.Description = todoNode["TODO"].as<std::string>();

			m_Notes.push_back(note);
		}
	}

	const ImVec4& TODOPanel::GetNoteColor(const Note& note)
	{
		switch (note.ImportanceLevel)
		{
			case Importance::Urgent: return s_UrgentNoteColor;
			case Importance::WhenPossible: return s_WhenPossibleNoteColor;
		}

		AR_CORE_ASSERT(false, "Unknown level!");
		return s_UrgentNoteColor;
	}

}