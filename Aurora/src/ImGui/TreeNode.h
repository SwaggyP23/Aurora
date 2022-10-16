#pragma once

#include "Core/Base.h"
#include "Graphics/Texture.h"

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

// TODO: Fix These...
namespace ImGui {
	
	bool TreeNodeWithIcon(Aurora::Ref<Aurora::Texture2D> icon, ImGuiID id, ImGuiTreeNodeFlags flags, const char* label, const char* label_end, ImColor iconTint = IM_COL32_WHITE);

	bool TreeNodeWithIcon(Aurora::Ref<Aurora::Texture2D> icon, const void* ptr_id, ImGuiTreeNodeFlags flags, ImColor iconTint, const char* fmt, ...);

	bool TreeNodeWithIcon(Aurora::Ref<Aurora::Texture2D> icon, const char* label, ImGuiTreeNodeFlags flags, ImColor tintColor = IM_COL32_WHITE);

}