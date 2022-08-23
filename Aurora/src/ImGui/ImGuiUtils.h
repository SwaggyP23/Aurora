#pragma once

#include "Theme.h"

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

// TODO: Refactor into a corresponding cpp file

namespace Aurora {

	namespace ImGuiUtils {

		static int s_UIContextID = 0;
		static uint32_t s_Counter = 0;
		static char s_IDBuffer[16] = "##";

		// Taken from Hazel-dev
		static const char* GenerateID()
		{
			_itoa_s(s_Counter++, s_IDBuffer + 2, sizeof(s_IDBuffer) - 2, 16);
			return s_IDBuffer;
		}

		static void PushID()
		{
			ImGui::PushID(s_UIContextID++);
			s_Counter = 0;
		}

		static void PopID()
		{
			ImGui::PopID();
			s_UIContextID--;
		}

		static void ShowHelpMarker(const char* description)
		{
			ImGui::TextDisabled("(?)");
			if (ImGui::IsItemHovered())
			{
				ImGui::BeginTooltip();
				ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
				ImGui::TextUnformatted(description);
				ImGui::PopTextWrapPos();
				ImGui::EndTooltip();
			}
		}

		static void ToolTip(const std::string& tip, const ImVec4& color = ImVec4(1.0f, 1.0f, 0.529f, 0.7f))
		{
			ImGui::BeginTooltip();
			ImGui::TextColored(color, tip.c_str());
			ImGui::EndTooltip();
		}

		template<typename... Args>
		static void ToolTipWithVariableArgs(const ImVec4& color, const std::string& tip, Args&&... args)
		{
			ImGui::BeginTooltip();
			ImGui::TextColored(color, tip.c_str(), std::forward<Args>(args)...);
			ImGui::EndTooltip();
		}

		static bool IsInputEnabled()
		{
			const ImGuiIO& io = ImGui::GetIO();
			return (io.ConfigFlags & ImGuiConfigFlags_NoMouse) == 0 && (io.ConfigFlags & ImGuiConfigFlags_NavNoCaptureKeyboard) == 0;
		}

		static void SetInputEnabled(bool state)
		{
			ImGuiIO& io = ImGui::GetIO();

			// Since ImGui provides only the negative options (no mouse and no nav keyboard) so we have to reverse the bit ops
			// In the meaning that we OR if we want to disable and we and with the complement of it to enable (DeMorgan's)
			if (state)
			{
				io.ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
				io.ConfigFlags &= ~ImGuiConfigFlags_NavNoCaptureKeyboard;
			}
			else
			{
				io.ConfigFlags |= ImGuiConfigFlags_NoMouse;
				io.ConfigFlags |= ImGuiConfigFlags_NavNoCaptureKeyboard;
			}
		}

		static bool IsMouseEnabled()
		{
			// AND with the opposite of no mouse, if both return true then the mouse is enabled
			return ImGui::GetIO().ConfigFlags & ~ImGuiConfigFlags_NoMouse;
		}

		static void SetMouseEnabled(bool enabled)
		{
			if (enabled)
			{
				ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
			}
			else
			{
				ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouse;
			}
		}

		static void ShiftCursorY(float distance)
		{
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + distance);
		}

		static void ShiftCursorX(float distance)
		{
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + distance);
		}

		//////// DrawList Utils /////////

		// This get the last items rect!
		static ImRect GetItemRect()
		{
			ImRect res = ImRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());

			return res;
		}
		
		// To be used inside a Begin()/End() range
		static ImRect GetWindowRect()
		{
			return { ImGui::GetWindowContentRegionMin(), ImGui::GetWindowContentRegionMax() };
		}

		static bool IsMouseInRectRegion(ImVec2 min, ImVec2 max, bool clip = true)
		{
			return ImGui::IsMouseHoveringRect(min, max, clip);
		}

		static bool IsMouseInRectRegion(ImRect rect, bool clip = true)
		{
			return ImGui::IsMouseHoveringRect(rect.Min, rect.Max, clip);
		}

		static ImRect RectExpanded(const ImRect& input, float x, float y)
		{
			ImRect res = input;
			res.Min.x -= x;
			res.Min.y -= y;
			res.Max.x += x;
			res.Max.y += y;

			return res;
		}

		static void DrawItemActivityOutline(float rounding = 0.0f, bool drawWhenNotActive = false, ImColor colorWhenActive = ImColor(80, 80, 80))
		{
			ImDrawList* drawList = ImGui::GetWindowDrawList();
			const ImRect rect = RectExpanded(GetItemRect(), 1.0f, 1.0f);
			if (ImGui::IsItemHovered() && !ImGui::IsItemActive())
			{
				drawList->AddRect(rect.Min, rect.Max, Theme::SelectionMuted, rounding, 0, 1.5f);
			}
			else if (ImGui::IsItemActive())
			{
				drawList->AddRect(rect.Min, rect.Max, colorWhenActive, rounding, 0, 1.5f);
			}
			else if (!ImGui::IsItemHovered() && drawWhenNotActive)
			{
				drawList->AddRect(rect.Min, rect.Max, Theme::BackgroundDark, rounding, 0, 1.5f);
			}
		}

		template<uint32_t BuffSize = 256, typename StringType>
		static bool SearchBox(StringType& searchString, const char* searchHint = "Search...")
		{
			// TODO: Continue WORK!
			PushID();

			bool modified = false;

			char buffer[BuffSize]{};
			strcpy_s<BuffSize>(buffer, searchString.c_str());
			if (ImGui::InputTextWithHint(GenerateID(), searchHint, buffer, sizeof(buffer)))
			{
				searchString = std::string(buffer);
				modified = true;
			}

			DrawItemActivityOutline(3.0f, true, Theme::Accent);

			//ShiftCursorY(1.0f);

			//const bool layoutSuspended = []()
			//{
			//	ImGuiWindow* window = ImGui::GetCurrentWindow();
			//	if (window->DC.CurrentLayout)
			//	{
			//		ImGui::SuspendLayout();

			//		return true;
			//	}

			//	return false;
			//}();

			//bool modified = false;
			//bool searching = false;

			//const float areaPosX = ImGui::GetCursorPosX();
			//const float framePaddingY = ImGui::GetStyle().FramePadding.y;

			//ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);
			//ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 28.0f, framePaddingY });

			//if constexpr (std::is_same<StringType, std::string>::value)
			//{
			//	char searchBuffer[BuffSize]{};
			//	strcpy_s<BuffSize>(searchBuffer, searchString.c_str());
			//	if (ImGui::InputText(GenerateID(), searchBuffer, BuffSize))
			//	{
			//		searchString = searchBuffer;
			//		modified = true;
			//	}
			//	else if (ImGui::IsItemDeactivatedAfterEdit())
			//	{
			//		searchString = searchBuffer;
			//		modified = true;
			//	}

			//	searching = searchBuffer[0] != 0;
			//}
			//else
			//{
			//	static_assert(std::is_same<decltype(&searchString[0]), char*>::value, "Search parameter must be a std::string or char*");

			//	if (ImGui::InputText(GenerateID(), searchString, BuffSize))
			//	{
			//		modified = true;
			//	}
			//	else if (ImGui::IsItemDeactivatedAfterEdit())
			//	{
			//		modified = true;
			//	}

			//	searching = searching[0] != 0;
			//}

			//DrawItemActivityOutline(3.0f, true, Theme::Accent);
			//ImGui::SetItemAllowOverlap();

			//ImGui::SameLine(areaPosX + 5.0f);

			//if (layoutSuspended)
			//	ImGui::ResumeLayout();

			//ImGui::BeginHorizontal(GenerateID(), ImGui::GetItemRectSize());

			//// Hint...
			//{
			//	if (!searching)
			//	{
			//		ShiftCursorY(-framePaddingY + 1.0f);
			//		ImGui::PushStyleColor(ImGuiCol_Text, Theme::TextDarker);
			//		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 0.0f, framePaddingY });

			//		ImGui::TextUnformatted(searchHint);
			//		ShiftCursorY(-1.0f);

			//		ImGui::PopStyleVar();
			//		ImGui::PopStyleColor();
			//	}
			//}

			//ImGui::Spring();

			//if (searching)
			//{
			//	const float spacingX = 4.0f;
			//	const float lineHeight = ImGui::GetItemRectSize().y - framePaddingY / 2.0f;

			//	if (ImGui::InvisibleButton(GenerateID(), ImVec2{ lineHeight, lineHeight }))
			//	{
			//		if constexpr (std::is_same<StringType, std::string>::value)
			//			searchString.clear();
			//		else
			//			memset(searchString, 0, BuffSize);

			//		modified = true;
			//	}

			//	if (ImGui::IsMouseHoveringRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax()))
			//		ImGui::SetMouseCursor(ImGuiMouseCursor_Arrow);

			//	ImGui::Spring(-1.0f, spacingX * 2.0f);
			//}

			//ImGui::EndHorizontal();
			//ShiftCursorY(-1.0f);

			//ImGui::PopStyleVar(2);

			PopID();

			return modified;
		}

	}

}