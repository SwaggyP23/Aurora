#pragma once

#include "Theme.h"
#include "FontAwesome.h"
#include "Editor/EditorResources.h"

#include <glm/glm.hpp>
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

namespace ImGui {

	IMGUI_API bool DragFloat2(const char* label, glm::vec2& v, float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f, const char* format = "%.3f", ImGuiSliderFlags flags = 0);
	IMGUI_API bool DragFloat3(const char* label, glm::vec3& v, float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f, const char* format = "%.3f", ImGuiSliderFlags flags = 0);
	IMGUI_API bool DragFloat4(const char* label, glm::vec4& v, float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f, const char* format = "%.3f", ImGuiSliderFlags flags = 0);

	IMGUI_API bool InputText(const char* label, std::string* str, ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = nullptr, void* user_data = nullptr);
	IMGUI_API bool InputTextMultiline(const char* label, std::string* str, const ImVec2& size = ImVec2(0, 0), ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = nullptr, void* user_data = nullptr);
	IMGUI_API bool InputTextWithHint(const char* label, const char* hint, std::string* str, ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = nullptr, void* user_data = nullptr);

}

namespace Aurora {

	struct ImGuiScopedStyle
	{
		ImGuiScopedStyle(ImGuiStyleVar var, float value) { ImGui::PushStyleVar(var, value); }
		ImGuiScopedStyle(ImGuiStyleVar var, const ImVec2& value) { ImGui::PushStyleVar(var, value); }
		~ImGuiScopedStyle() { ImGui::PopStyleVar(); }
	};

	struct ImGuiScopedColor
	{
		ImGuiScopedColor(ImGuiCol var, const ImVec4& value) { ImGui::PushStyleColor(var, value); }
		ImGuiScopedColor(ImGuiCol var, const ImU32& value) { ImGui::PushStyleColor(var, value); }
		~ImGuiScopedColor() { ImGui::PopStyleColor(); }
	};

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

		bool IsWindowFocused(const char* windowName, bool checkRootWindow = true);

		// Colors...
		ImColor ColorWithMultiplierValue(const ImColor& color, float multi);

		ImColor ColourWithMultipliedSaturation(const ImColor& color, float multiplier);

		bool ColorEdit3Control(const char* label, glm::vec3& color, bool showAsWheel = true);

		bool ColorEdit4Control(const char* label, glm::vec4& color, bool showAsWheel = true);

		//////// DrawList Utils /////////
		// This get the last items rect!
		ImRect GetItemRect();
		
		// To be used inside a Begin()/End() range
		ImRect GetWindowRect(bool clip = false);

		bool IsMouseInRectRegion(ImVec2 min, ImVec2 max, bool clip = true);

		bool IsMouseInRectRegion(ImRect rect, bool clip = true);

		ImRect RectExpanded(const ImRect& input, float x, float y);

		ImRect RectOffset(const ImRect& input, float x, float y);

		bool IsItemDisabled();

		void DrawItemActivityOutline(float rounding = 0.0f, bool drawWhenNotActive = false, ImColor colorWhenActive = ImColor(80, 80, 80));

		void DrawButtonImage(const Ref<Texture2D>& texture, ImColor tintNormal, ImColor tintHovered, ImColor tintActive, ImRect rect);

		void DrawButtonImage(const char* glyph, ImColor tintNormal, ImColor tintHovered, ImColor tintActive, ImRect rect);

		void UnderLine(bool fullWidth = false, float offsetX = 0.0f, float offsetY = -1.0f);

		template<std::size_t BuffSize = 256, typename StringType>
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

				DrawButtonImage(AR_ICON_TIMES_CIRCLE_O,
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

		template<typename F>
		void Table(const char* name, const char** columns, uint32_t columnCount, const ImVec2& size, const F& callback)
		{
			if (size.x == 0.0f || size.y == 0.0f)
				return;

			constexpr float edgeOffset = 4.0f;

			ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2{ 4.0f, 0.0f });
			ImColor backgroundColor = ImColor(Theme::BackgroundDark);
			const ImColor colRowAlt = ColorWithMultiplierValue(backgroundColor, 1.3f);
			ImGui::PushStyleColor(ImGuiCol_TableRowBg, backgroundColor.Value);
			ImGui::PushStyleColor(ImGuiCol_TableRowBgAlt, colRowAlt.Value);
			ImGui::PushStyleColor(ImGuiCol_ChildBg, backgroundColor.Value);

			constexpr ImGuiTableFlags flags = ImGuiTableFlags_NoPadInnerX
				| ImGuiTableFlags_Resizable
				//| ImGuiTableFlags_Reorderable
				| ImGuiTableFlags_ScrollY
				| ImGuiTableFlags_RowBg;

			if (!ImGui::BeginTable(name, columnCount, flags, size))
				return;

			const float cursorX = ImGui::GetCursorScreenPos().x;

			for (uint32_t i = 0; i < columnCount; i++)
				ImGui::TableSetupColumn(columns[i]);

			{
				ImU32 headerColor = ImGuiUtils::ColorWithMultiplierValue(Theme::AccentDimmed, 1.2f);
				ImGui::PushStyleColor(ImGuiCol_HeaderHovered, headerColor);
				ImGui::PushStyleColor(ImGuiCol_HeaderActive, headerColor);
				ImGui::TableSetupScrollFreeze(ImGui::TableGetColumnCount(), 1);

				ImGui::TableNextRow(ImGuiTableRowFlags_Headers, 22.0f);
				for (uint32_t i = 0; i < columnCount; i++)
				{
					ImGui::TableSetColumnIndex(i);
					const char* columnName = ImGui::TableGetColumnName(i);
					ImGui::PushID(columnName);

					ShiftCursor(edgeOffset * 3.0f, edgeOffset * 2.0f);
					ImGui::TableHeader(columnName, 0, Theme::Background);
					ShiftCursor(-edgeOffset * 3.0f, -edgeOffset * 2.0f);

					ImGui::PopID();
				}
				ImGui::SetCursorScreenPos(ImVec2(cursorX, ImGui::GetCursorScreenPos().y));
				UnderLine(true, 0.0f, 5.0f);

				ImGui::PopStyleColor(2);
			}

			callback();

			ImGui::EndTable();

			ImGui::PopStyleColor(3);
			ImGui::PopStyleVar();
		}

		bool TableRowClickable(const char* id, float rowHeight);

		// UI...
		bool PropertyGridHeader(const std::string& name, bool openByDefault = true);

		void BeginPropertyGrid(uint32_t columns = 2, bool defaultWidth = true);

		void EndPropertyGrid();

		void Separator(ImVec2 size, ImVec4 color);

		bool PropertyString(const char* label, const char* value, bool isError = false);

		bool PropertyStringReadOnly(const char* label, const char* value, bool isErorr = false);

		bool PropertyFloat(const char* label, float& value, float delta = 0.1f, float min = 0.0f, float max = 0.0f, const char* helpText = "");

		bool PropertyBool(const char* label, bool& value, const char* helpText = "");

		template<typename TEnum, typename TType = int32_t>
		bool PropertyDropdown(const char* label, const char** options, int optionCount, TEnum& selected, const char* helpText = "")
		{
			bool modified = false;
			TType selectedIndex = (TType)selected;
			const char* current = options[selectedIndex];

			ShiftCursor(10.0f, 9.0f);
			ImGui::Text(label);

			if (std::strlen(helpText) != 0)
			{
				ImGui::SameLine();
				ShowHelpMarker(helpText);
			}

			ImGui::NextColumn();
			ShiftCursorY(4.0f);

			ImGui::PushItemWidth(-1);
			const std::string id = fmt::format("##{0}", label);
			if (ImGui::BeginCombo(id.c_str(), current))
			{
				for (int i = 0; i < optionCount; i++)
				{
					bool isSelected = (current == options[i]);
					if (ImGui::Selectable(options[i], isSelected))
					{
						current = options[i];
						*selected = (TType)i;
						modified = true;
					}

					if (isSelected)
						ImGui::SetItemDefaultFocus();
				}

				ImGui::EndCombo();
			}

			if (!IsItemDisabled())
				DrawItemActivityOutline(2.0f, true, Theme::Accent);

			ImGui::PopItemWidth();
			ImGui::NextColumn();
			UnderLine();

			return modified;
		}

		bool PropertyDropdown(const char* label, const char** options, int optionCount, int* selected, const char* helpText = "");

		bool PropertySliderFloat(const char* label, float& value, float min = 0.0f, float max = 0.0f, const char* format = "%.3f", const char* helpText = "");

		bool PropertySliderFloat2(const char* label, ImVec2& value, float min = 0.0f, float max = 0.0f, const char* format = "%.3f", const char* helpText = "");

		bool MultiLineText(const char* label, std::string& value);

		//void BeginPropertyCheckBoxGroup(const char* label);

		//bool PropertyCheckBoxGroup(const char* label, bool& value);

		//void EndPropertyCheckBoxGroup();

		// Custom TreeNodes...
		bool TreeNodeWithIcon(const char* label, Ref<Texture2D> icon, const ImVec2& size, bool openByDefault = true);
	}

}