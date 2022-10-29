#pragma once

#include "EditorPanel.h"

#include <imgui/imgui.h>

namespace Aurora {

	class TODOPanel : public EditorPanel
	{
	public:
		enum class Importance : int16_t
		{
			None = 0, Urgent = 1, WhenPossible = 2, All = Urgent | WhenPossible
		};

		struct Note
		{
			Importance ImportanceLevel;
			std::string Description;
		};

	public:
		TODOPanel();
		virtual ~TODOPanel();

		static Ref<TODOPanel> Create();

		virtual void OnImGuiRender(bool& isOpen) override;
		virtual void OnEvent(Event& e) override;

	private:
		void AddNote(const Note& note);
		void RemoveNote(int indexToRemoveAt);

		void Serialize();
		void Deserialize();

		const ImVec4& GetNoteColor(const Note& note);

	private:
		std::vector<Note> m_Notes;
		int16_t m_NoteFilters = (int16_t)Importance::All;

		bool m_TypePanelOpen = false;
		bool m_DetailedPanelOpen = false;

	};

}