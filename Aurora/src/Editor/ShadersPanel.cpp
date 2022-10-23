#include "Aurorapch.h"
#include "ShadersPanel.h"

#include "Core/Input/Input.h"
#include "ImGui/ImGuiUtils.h"
#include "Graphics/Shader.h"

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

namespace Aurora {

	static bool s_ActivateSearchWidget = false;

	Ref<ShadersPanel> ShadersPanel::Create()
	{
		return CreateRef<ShadersPanel>();
	}

	// TODO: Change to the following...
	// - Name:
	// - Path:
	// - Type:
	// - LastTimeModified:
	// - Reload:?
	void ShadersPanel::OnImGuiRender(bool& isOpen)
	{
		ImGui::Begin("Shaders", &isOpen);

		constexpr float edgeOffset = 4.0f;
		ImGuiUtils::ShiftCursorX(edgeOffset * 3.0f);
		ImGuiUtils::ShiftCursorY(edgeOffset * 2.0f);

		ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - edgeOffset * 3.0f);

		if (s_ActivateSearchWidget)
		{
			ImGui::SetKeyboardFocusHere();
			s_ActivateSearchWidget = false;
		}

		static std::string searchString;
		ImGuiUtils::SearchBox(searchString);

		ImGui::Spacing();
		ImGui::Spacing();

		ImGuiUtils::BeginPropertyGrid();

		for (Ref<Shader> shader : Shader::AllShaders)
		{
			const std::string& shaderName = shader->GetName();

			if (!ImGuiUtils::IsMatchingSearch(shaderName, searchString))
				continue;

			ImGui::PushID(fmt::format("shader{0}", shaderName).c_str());

			ImGuiUtils::PropertyStringReadOnly("Name", shaderName.c_str());
			ImGuiUtils::PropertyStringReadOnly("- Path", shader->GetFilePath().string().c_str());
			ImGuiUtils::PropertyStringReadOnly("- Type", shader->GetTypeString().c_str());
			uint32_t lastTimeMod = shader->GetLastTimeModified();
			ImGuiUtils::PropertyStringReadOnly("- Last Modified", (std::to_string(lastTimeMod) + " Minute(s) ago").c_str(), lastTimeMod <= Shader::GetCompileTimeThreshold());
			
			ImGuiUtils::ShiftCursor(10.0f, 9.0f);

			if (ImGui::Button("Reload"))
				shader->Reload();

			bool compilationFailed = shader->CompilationFailed();
			ImGui::NextColumn();
			ImGuiUtils::ShiftCursorY(4.0f);

			ImGui::PushItemWidth(-1);
			std::string message = compilationFailed ? "Compilation Error! Check Console for more info. Reverting to Old Shader!" : "";
			if (compilationFailed)
				ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(204, 51, 76.5, 255));
			ImGui::InputText(fmt::format("##shader{0}compilationFailed", shaderName).c_str(), (char*)message.c_str(), 0, ImGuiInputTextFlags_ReadOnly);
			if (compilationFailed)
				ImGui::PopStyleColor();
			ImGui::PopItemWidth();

			ImGui::NextColumn();
			ImGui::Separator();

			ImGui::PopID();
		}

		ImGuiUtils::EndPropertyGrid();

		ImGui::End();
	}

	void ShadersPanel::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);

		dispatcher.Dispatch<KeyPressedEvent>([this](KeyPressedEvent& event) {return OnKeyPressed(event); });
	}

	bool ShadersPanel::OnKeyPressed(KeyPressedEvent& e)
	{
		if (Input::IsKeyDown(KeyCode::LeftControl))
		{
			switch (e.GetKeyCode())
			{
			    case KeyCode::F:
			    {
			    	s_ActivateSearchWidget = true;
			    	break;
			    }
			}
		}

		return false;
	}

}