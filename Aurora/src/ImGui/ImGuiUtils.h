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