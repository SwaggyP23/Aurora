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

		static char* s_MultiLineBuffer = nullptr;
		static uint32_t s_MultiLineBufferSize = 1024 * 1024; // 1KB

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

		// Prefer using this over the ToolTip
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

		bool IsItemDisabled()
		{
			return ImGui::GetItemFlags() & ImGuiItemFlags_Disabled;
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

		void UnderLine(bool fullWidt, float offsetX, float offsetY)
		{
			if (fullWidt)
			{
				if (ImGui::GetCurrentWindow()->DC.CurrentColumns != nullptr)
					ImGui::PushColumnsBackground();
				else if (ImGui::GetCurrentTable() != nullptr)
					ImGui::TablePushBackgroundChannel();
			}

			const float width = fullWidt ? ImGui::GetWindowWidth() : ImGui::GetContentRegionAvail().x;
			const ImVec2 cursor = ImGui::GetCursorScreenPos();
			ImGui::GetWindowDrawList()->AddLine(ImVec2{ cursor.x + offsetX, cursor.y + offsetY }, ImVec2{cursor.x + width, cursor.y + offsetY}, Theme::BackgroundDark, 1.0f);

			if (fullWidt)
			{
				if (ImGui::GetCurrentWindow()->DC.CurrentColumns != nullptr)
					ImGui::PopColumnsBackground();
				else if (ImGui::GetCurrentTable() != nullptr)
					ImGui::TablePopBackgroundChannel();
			}
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

		bool ColorEdit3Control(const char* label, glm::vec3& color, bool showAsWheel)
		{
			ImGuiColorEditFlags flags = ImGuiColorEditFlags_AlphaBar
				| ImGuiColorEditFlags_AlphaPreview
				| ImGuiColorEditFlags_HDR
				| (showAsWheel ? ImGuiColorEditFlags_PickerHueWheel : ImGuiColorEditFlags_PickerHueBar);

			ShiftCursor(10.0f, 9.0f);
			ImGui::Text(label);
			ImGui::NextColumn();
			ShiftCursorY(4.0f);

			ImGui::PushItemWidth(-1);
			bool modified = ImGui::ColorEdit3(fmt::format("##{0}", label).c_str(), glm::value_ptr(color), flags);
			ImGui::PopItemWidth();

			ImGui::NextColumn();
			UnderLine();

			return modified;
		}

		bool ColorEdit4Control(const char* label, glm::vec4& color, bool showAsWheel)
		{
			ImGuiColorEditFlags flags = ImGuiColorEditFlags_AlphaBar
									  | ImGuiColorEditFlags_AlphaPreview
									  | ImGuiColorEditFlags_HDR
									  | (showAsWheel ? ImGuiColorEditFlags_PickerHueWheel : ImGuiColorEditFlags_PickerHueBar);

			ShiftCursor(10.0f, 9.0f);
			ImGui::Text(label);
			ImGui::NextColumn();
			ShiftCursorY(4.0f);

			ImGui::PushItemWidth(-1);
			bool modified = ImGui::ColorEdit4(fmt::format("##{0}", label).c_str(), glm::value_ptr(color), flags);
			ImGui::PopItemWidth();

			ImGui::NextColumn();
			UnderLine();

			return modified;
		}

		// UI...
		bool PropertyGridHeader(const std::string& name, bool openByDefault)
		{
			ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_Framed
				| ImGuiTreeNodeFlags_SpanAvailWidth
				| ImGuiTreeNodeFlags_AllowItemOverlap
				| ImGuiTreeNodeFlags_FramePadding;

			if (openByDefault)
				treeNodeFlags |= ImGuiTreeNodeFlags_DefaultOpen;
			
			constexpr float framePaddingX = 6.0f;
			constexpr float framePaddingY = 6.0f;

			ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ framePaddingX, framePaddingY });
			ImGui::PushID(name.c_str());
			bool open = ImGui::TreeNodeEx("##dummy_id", treeNodeFlags, Utils::StringUtils::ToUpper(name).c_str());
			ImGui::PopID();
			ImGui::PopStyleVar(2);

			return open;
		}

		void BeginPropertyGrid(uint32_t columns, bool defaultWidth)
		{
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 8.0f, 8.0f });
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4.0f, 4.0f });
			PushID();
			ImGui::Columns(columns);
			if(defaultWidth)
				ImGui::SetColumnWidth(0, 140.0f);
		}

		void EndPropertyGrid()
		{
			ImGui::Columns(1);
			ImGui::PopStyleVar(2);
			ShiftCursorY(8.0f);
			PopID();
		}

		void Separator(ImVec2 size, ImVec4 color)
		{
			ImGui::PushStyleColor(ImGuiCol_ChildBg, color);
			ImGui::BeginChild("sep", size);
			ImGui::EndChild();
			ImGui::PopStyleColor();
		}

		bool PropertyFloat(const char* label, float& value, float delta, float min, float max, const char* helpText)
		{
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
			bool modified = ImGui::DragFloat(fmt::format("##{0}", label).c_str(), &value, delta, min, max);

			if (!IsItemDisabled())
				DrawItemActivityOutline(2.0f, true, Theme::Accent);
			ImGui::PopItemWidth();

			ImGui::NextColumn();
			UnderLine();

			return modified;
		}

		bool PropertyBool(const char* label, bool& value, const char* helpText)
		{
			ShiftCursor(10.0f, 9.0f);
			ImGui::Text(label);

			if (std::strlen(helpText) != 0)
			{
				ImGui::SameLine();
				ShowHelpMarker(helpText);
			}

			ImGui::NextColumn();
			ShiftCursorY(4.0f);

			bool modified = ImGui::Checkbox(fmt::format("##{0}", label).c_str(), &value);

			// TODO: Maybe useless for the checkbox
			if (!IsItemDisabled())
				DrawItemActivityOutline(2.0f, true, Theme::Accent);

			ImGui::NextColumn();
			UnderLine();

			return modified;
		}

		bool PropertyDropdown(const char* label, const char** options, int optionCount, int* selected, const char* helpText)
		{
			bool modified = false;
			const char* current = options[*selected];

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
						*selected = i;
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

		bool PropertySliderFloat(const char* label, float& value, float min, float max, const char* format, const char* helpText)
		{
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
			bool modified = ImGui::SliderFloat(fmt::format("##{0}", label).c_str(), &value, min, max, format);

			if (!IsItemDisabled())
				DrawItemActivityOutline(2.0f, true, Theme::Accent);
			ImGui::PopItemWidth();

			ImGui::NextColumn();
			UnderLine();

			return modified;
		}

		bool PropertySliderFloat2(const char* label, ImVec2& value, float min, float max, const char* format, const char* helpText)
		{
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
			bool modified = ImGui::SliderFloat2(fmt::format("##{0}", label).c_str(), (float*)&value, min, max, format);

			if (!IsItemDisabled())
				DrawItemActivityOutline(2.0f, true, Theme::Accent);
			ImGui::PopItemWidth();

			ImGui::NextColumn();
			UnderLine();

			return modified;
		}

		bool MultiLineText(const char* label, std::string& value)
		{
			bool modified = false;

			ImGui::Text(label);
			ImGui::NextColumn();

			if (!s_MultiLineBuffer)
			{
				s_MultiLineBuffer = new char[s_MultiLineBufferSize];
				memset(s_MultiLineBuffer, 0, s_MultiLineBufferSize);
			}

			strcpy_s(s_MultiLineBuffer, s_MultiLineBufferSize, value.c_str());

			ImGui::PushItemWidth(-1);
			if (ImGui::InputTextMultiline(fmt::format("##{0}", label).c_str(), s_MultiLineBuffer, s_MultiLineBufferSize))
			{
				value = s_MultiLineBuffer;
				modified = true;
			}
			ImGui::PopItemWidth();

			ImGui::NextColumn();

			return modified;
		}

		//static int s_CheckBoxCount = 0;

		//void BeginPropertyCheckBoxGroup(const char* label)
		//{
		//	ShiftCursor(10.0f, 9.0f);
		//	ImGui::Text(label);
		//	ImGui::NextColumn();
		//}

		//bool PropertyCheckBoxGroup(const char* label, bool& value)
		//{
		//	bool modified = false;

		//	if (++s_CheckBoxCount > 1)
		//		ImGui::SameLine();
		//	else
		//		ShiftCursor(10.0f, 9.0f);

		//	ImGui::Text(label);
		//	ImGui::SameLine();

		//	if (ImGui::Checkbox(GenerateID(), &value))
		//		modified = true;

		//	if (!IsItemDisabled())
		//		DrawItemActivityOutline(2.0f, true, Theme::Accent);

		//	return modified;
		//}

		//void EndPropertyCheckBoxGroup()
		//{
		//	ImGui::NextColumn();
		//	s_CheckBoxCount = 0;
		//}

		// Custom TreeNodes...
		bool TreeNodeWithIcon(const char* label, Ref<Texture2D> icon, const ImVec2& size, bool openByDefault)
		{
			ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_Framed
				| ImGuiTreeNodeFlags_SpanAvailWidth
				| ImGuiTreeNodeFlags_AllowItemOverlap
				| ImGuiTreeNodeFlags_FramePadding;

			if (openByDefault)
				treeNodeFlags |= ImGuiTreeNodeFlags_DefaultOpen;

			constexpr float framePaddingX = 6.0f;
			constexpr float framePaddingY = 6.0f; // affects height of the header

			ImGui::PushID(label);
			bool open = ImGui::TreeNodeEx("##dummy_id", treeNodeFlags, label);

			float lineHeight = ImGui::GetItemRectMax().y - ImGui::GetItemRectMin().y;
			ImGui::SameLine();

			ShiftCursorY(size.y / 4.0f);
			ImGui::Image((void*)(uint64_t)icon->GetTextureID(), size, ImVec2{ 0, 0 }, ImVec2{ 1, 1 });
			ShiftCursorY(-(size.y / 4.0f));

			ImGui::PopID();
			
			return open;
		}
		
	}

}