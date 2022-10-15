#include "Aurorapch.h"
#include "ImGuiUtils.h"

#include "ImGuizmo.h"
#include "Utils/UtilFunctions.h"

#include <choc/text/choc_StringUtilities.h>
#include <glm/gtc/type_ptr.hpp>

namespace Aurora {

	namespace ImGuiUtils {

		static int s_UIContextID = 0;
		static uint32_t s_Counter = 0;
		static char s_IDBuffer[16] = "##";

		// Taken from Hazel-dev
		const char* GenerateID()
		{
			_itoa_s(s_Counter++, s_IDBuffer + 2, sizeof(s_IDBuffer) - 2, 16);
			return s_IDBuffer;
		}

		void PushID()
		{
			ImGui::PushID(s_UIContextID++);
			s_Counter = 0;
		}

		void PopID()
		{
			ImGui::PopID();
			s_UIContextID--;
		}

		void ShowHelpMarker(const char* description)
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

		void ToolTip(const std::string& tip, const ImVec4& color)
		{
			ImGui::BeginTooltip();
			ImGui::TextColored(color, tip.c_str());
			ImGui::EndTooltip();
		}

		void SetGuizmoEnabled(bool state)
		{
			ImGuizmo::Enable(state);
		}

		bool IsInputEnabled()
		{
			const ImGuiIO& io = ImGui::GetIO();
			return (io.ConfigFlags & ImGuiConfigFlags_NoMouse) == 0 && (io.ConfigFlags & ImGuiConfigFlags_NavNoCaptureKeyboard) == 0;
		}

		void SetInputEnabled(bool state)
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

		bool IsMouseEnabled()
		{
			// AND with the opposite of no mouse, if both return true then the mouse is enabled
			return ImGui::GetIO().ConfigFlags & ~ImGuiConfigFlags_NoMouse;
		}

		void SetMouseEnabled(bool enabled)
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

		void ShiftCursor(float x, float y)
		{
			ImVec2 cursorPos = ImGui::GetCursorPos();
			ImGui::SetCursorPos(ImVec2{ cursorPos.x + x, cursorPos.y + y });
		}

		void ShiftCursorY(float distance)
		{
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + distance);
		}

		void ShiftCursorX(float distance)
		{
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + distance);
		}

		bool NavigatedTo()
		{
			return GImGui->NavJustMovedToId == GImGui->LastItemData.ID;
		}

		//////// DrawList Utils /////////

		// This get the last items rect!
		ImRect GetItemRect()
		{
			ImRect res = ImRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());

			return res;
		}

		// To be used inside a Begin()/End() range
		// This returns the inner rect of the window because so far that is the most useful rect to use for UI imgui stuff
		ImRect GetWindowRect(bool clip)
		{
			if (clip)
				return GImGui->CurrentWindow->InnerClipRect;
			else
				return GImGui->CurrentWindow->InnerRect;
		}

		bool IsMouseInRectRegion(ImVec2 min, ImVec2 max, bool clip)
		{
			return ImGui::IsMouseHoveringRect(min, max, clip);
		}

		bool IsMouseInRectRegion(ImRect rect, bool clip)
		{
			return ImGui::IsMouseHoveringRect(rect.Min, rect.Max, clip);
		}

		ImRect RectExpanded(const ImRect& input, float x, float y)
		{
			ImRect res = input;
			res.Min.x -= x;
			res.Min.y -= y;
			res.Max.x += x;
			res.Max.y += y;

			return res;
		}

		ImRect RectOffset(const ImRect& input, float x, float y)
		{
			ImRect res = input;
			res.Min.x += x;
			res.Min.y += y;
			res.Max.x += x;
			res.Max.y += y;
			return res;
		}

		void DrawItemActivityOutline(float rounding, bool drawWhenNotActive, ImColor colorWhenActive)
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

		void DrawButtonImage(const Ref<Texture2D>& texture, ImColor tintNormal, ImColor tintHovered, ImColor tintActive, ImRect rect)
		{
			ImDrawList* drawlist = ImGui::GetWindowDrawList();

			void* textureID = (void*)(uint64_t)texture->GetTextureID();

			if (ImGui::IsItemActive())
				drawlist->AddImage(textureID, rect.Min, rect.Max, ImVec2{ 0, 0 }, ImVec2{ 1, 1 }, tintActive);
			else if(ImGui::IsItemHovered())
				drawlist->AddImage(textureID, rect.Min, rect.Max, ImVec2{ 0, 0 }, ImVec2{ 1, 1 }, tintHovered);
			else
				drawlist->AddImage(textureID, rect.Min, rect.Max, ImVec2{ 0, 0 }, ImVec2{ 1, 1 }, tintNormal);
		}

		bool IsMatchingSearch(const std::string& name, std::string_view searchQuery, bool caseSensitive, bool noWhiteSpaces, bool noUnderScores)
		{
			// In case we are not searching, it makes sense to display everything
			if (searchQuery.empty())
				return true;

			// Item to search for should not be an empty string
			if (name.empty())
				return false;

			std::string nameSanitized = noUnderScores ? choc::text::replace(name, "_", "") : name;
			nameSanitized = noWhiteSpaces ? choc::text::replace(nameSanitized, " ", "") : nameSanitized;
			std::string searchString = noWhiteSpaces ? choc::text::replace(searchQuery, " ", "") : std::string(searchQuery);

			if (!caseSensitive)
			{
				nameSanitized = Utils::StringUtils::ToLower(nameSanitized);
				searchString = Utils::StringUtils::ToLower(searchString);
			}

			bool returnState = false;

			// The case where the input string contains white spaces means that there are multiple words therefore
			// i would say its sufficient to only start looking from the first token
			if (choc::text::contains(searchString, " "))
			{
				std::vector<std::string> stringTokens = choc::text::splitAtWhitespace(searchString);

				for (const std::string& token : stringTokens)
				{
					if (!token.empty() && choc::text::contains(nameSanitized, token))
						returnState = true;
					else
					{
						returnState = false;
						break;
					}
				}
			}
			else
			{
				returnState = choc::text::contains(nameSanitized, searchString);
			}

			return returnState;
		}

		// Colors...
		ImColor ColorWithMultiplierValue(const ImColor& color, float multi)
		{
			ImVec4 col = color.Value;
			float hue, sat, val;
			ImGui::ColorConvertRGBtoHSV(col.x, col.y, col.z, hue, sat, val);
			return ImColor::HSV(hue, sat, val, std::min(val * multi, 1.0f));
		}

		ImColor ColourWithMultipliedSaturation(const ImColor& color, float multiplier)
		{
			const ImVec4& colRaw = color.Value;
			float hue, sat, val;
			ImGui::ColorConvertRGBtoHSV(colRaw.x, colRaw.y, colRaw.z, hue, sat, val);
			return ImColor::HSV(hue, std::min(sat * multiplier, 1.0f), val);
		}

		void ColorEdit3Control(const std::string& label, glm::vec3& color, bool showAsWheel, float columnwidth)
		{
			ImGuiColorEditFlags flags = ImGuiColorEditFlags_AlphaBar
				| ImGuiColorEditFlags_AlphaPreview
				| ImGuiColorEditFlags_HDR
				| (showAsWheel ? ImGuiColorEditFlags_PickerHueWheel : ImGuiColorEditFlags_PickerHueBar);

			ImGui::Columns(2);
			ImGui::SetColumnWidth(0, columnwidth);

			ImGui::Text(label.c_str());

			ImGui::NextColumn();

			ImGui::PushItemWidth(-1);
			ImGui::ColorEdit3(GenerateID(), glm::value_ptr(color), flags);
			ImGui::PopItemWidth();

			ImGui::Columns(1);
		}

		void ColorEdit4Control(const std::string& label, glm::vec4& color, bool showAsWheel, float columnwidth)
		{
			ImGuiColorEditFlags flags = ImGuiColorEditFlags_AlphaBar
									  | ImGuiColorEditFlags_AlphaPreview
									  | ImGuiColorEditFlags_HDR
									  | (showAsWheel ? ImGuiColorEditFlags_PickerHueWheel : ImGuiColorEditFlags_PickerHueBar);

			ImGui::Columns(2);
			ImGui::SetColumnWidth(0, columnwidth);

			ImGui::Text(label.c_str());

			ImGui::NextColumn();

			ImGui::PushItemWidth(-1);
			ImGui::ColorEdit4(GenerateID(), glm::value_ptr(color), flags);
			ImGui::PopItemWidth();

			ImGui::Columns(1);
		}

	}

}