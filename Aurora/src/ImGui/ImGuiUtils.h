#pragma once

#include <imgui/imgui.h>

namespace Aurora {

	namespace ImGuiUtils {

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

	}

}