#pragma once

#include "Theme.h"
#include "Editor/EditorResources.h"

#include <glm/glm.hpp>
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

// TODO: Refactor into a corresponding cpp file

namespace Aurora {

	namespace ImGuiUtils {

		// Taken from Hazel-dev
		// https://hazelengine.com/
		const char* GenerateID();

		void PushID();

		void PopID();

		void ShowHelpMarker(const char* description);

		void ToolTip(const std::string& tip, const ImVec4& color = ImVec4(1.0f, 1.0f, 0.529f, 0.7f));

		template<typename... Args>
		void ToolTipWithVariableArgs(const ImVec4& color, const std::string& tip, Args&&... args)
		{
			ImGui::BeginTooltip();
			ImGui::TextColored(color, tip.c_str(), std::forward<Args>(args)...);
			ImGui::EndTooltip();
		}

		void SetGuizmoEnabled(bool state);

		bool IsInputEnabled();

		void SetInputEnabled(bool state);

		bool IsMouseEnabled();

		void SetMouseEnabled(bool enabled);

		void ShiftCursor(float x, float y);
		
		void ShiftCursorY(float distance);

		void ShiftCursorX(float distance);

		bool NavigatedTo();

		//////// DrawList Utils /////////
		// This get the last items rect!
		ImRect GetItemRect();
		
		// To be used inside a Begin()/End() range
		ImRect GetWindowRect(bool clip = false);

		bool IsMouseInRectRegion(ImVec2 min, ImVec2 max, bool clip = true);

		bool IsMouseInRectRegion(ImRect rect, bool clip = true);

		ImRect RectExpanded(const ImRect& input, float x, float y);

		ImRect RectOffset(const ImRect& input, float x, float y);

		void DrawItemActivityOutline(float rounding = 0.0f, bool drawWhenNotActive = false, ImColor colorWhenActive = ImColor(80, 80, 80));

		void DrawButtonImage(const Ref<Texture2D>& texture, ImColor tintNormal, ImColor tintHovered, ImColor tintActive, ImRect rect);

		template<uint32_t BuffSize = 256, typename StringType>
		static bool SearchBox(StringType& searchString, const char* searchHint = "Search...")
		{
			PushID();

			ShiftCursorY(1.0f);

			bool modified = false;
			bool searching = false;

			const float areaPosX = ImGui::GetCursorPosX();
			const float framePaddingY = ImGui::GetStyle().FramePadding.y;

			ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 28.0f, framePaddingY });

			char buffer[BuffSize]{};
			strcpy_s<BuffSize>(buffer, searchString.c_str());
			if (ImGui::InputTextWithHint(GenerateID(), searchHint, buffer, sizeof(buffer)))
			{
				searchString = std::string(buffer);
				modified = true;
			}

			searching = buffer[0] != 0;

			DrawItemActivityOutline(3.0f, true, Theme::Accent);
			ImGui::SetItemAllowOverlap();
			ImGui::SameLine(areaPosX + 5.0f);

			ImGui::BeginHorizontal(GenerateID(), ImGui::GetItemRectSize());
			const ImVec2 iconSize = ImVec2{ ImGui::GetTextLineHeight(), ImGui::GetTextLineHeight() };

			// Search Icon...
			{
				const float iconOffset = framePaddingY - 3.0f;
				ShiftCursorY(iconOffset);
				uint32_t textureID = EditorResources::SearchIcon->GetTextureID();
				ImGui::Image((void*)(uint64_t)textureID, iconSize, ImVec2{ 0, 0 }, ImVec2{ 1, 1, }, ImVec4{ 1.0f, 1.0f, 1.0f, 0.2f });
				ShiftCursorY(-iconOffset);
			}

			// This sets the position of the next button in the end of the search bar. If this is not here then the pos of clear
			// icon would be right next to the Search icon!
			ImGui::Spring();

			if (searching)
			{
				const float spacingX = 4.0f;
				const float lineHeight = ImGui::GetItemRectSize().y - framePaddingY / 2.0f;

				if (ImGui::InvisibleButton(GenerateID(), ImVec2{ lineHeight, lineHeight }))
				{
					if constexpr (std::is_same<StringType, std::string>::value)
						searchString.clear();
					else
						memset(searchString, 0, BuffSize);

					modified = true;
				}

				DrawButtonImage(EditorResources::ClearIcon, 
					ImColor(255, 255, 255, 51),
					ImColor(255, 255, 255, 171),
					ImColor(255, 255, 255, 128),
					RectExpanded(GetItemRect(), -2.0f, -2.0f));

				ImGui::Spring(-1.0f, spacingX * 2.0f);
			}
			
			ImGui::EndHorizontal();
			ShiftCursorY(-1.0f);
			ImGui::PopStyleVar(2);

			PopID();

			return modified;
		}

		bool IsMatchingSearch(const std::string& name, std::string_view searchQuery, bool caseSensitive = false, bool noWhiteSpaces = false, bool noUnderScores = false);

		// Colors...
		ImColor ColorWithMultiplierValue(const ImColor& color, float multi);

		ImColor ColourWithMultipliedSaturation(const ImColor& color, float multiplier);

		void ColorEdit3Control(const std::string& label, glm::vec3& color, bool showAsWheel = true);

		void ColorEdit4Control(const std::string& label, glm::vec4& color, bool showAsWheel = true);

	}

}