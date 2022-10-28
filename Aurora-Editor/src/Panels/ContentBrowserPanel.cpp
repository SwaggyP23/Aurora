#include "ContentBrowserPanel.h"

#include "ImGui/ImGuiUtils.h"

namespace Aurora {

	Ref<ContentBrowserPanel> ContentBrowserPanel::Create()
	{
		return CreateRef<ContentBrowserPanel>();
	}

	void ContentBrowserPanel::OnImGuiRender(bool& isOpen)
	{
		if (ImGui::Begin("Content Browser", &isOpen))
		{
			ImVec2 contentBrowserSize = ImGui::GetContentRegionAvail();
			contentBrowserSize.y -= 32.0f;

			RenderMenu({ contentBrowserSize.x, 28.0f });
			RenderBrowser(contentBrowserSize);
		}

		ImGui::End();
	}

	void ContentBrowserPanel::OnEvent(Event& e)
	{
	}

	void ContentBrowserPanel::RenderMenu(const ImVec2& size)
	{
		ImGuiScopedStyle frameBorderSize(ImGuiStyleVar_FrameBorderSize, 0.0f);
		ImGuiScopedStyle windowPadding(ImGuiStyleVar_WindowPadding, ImVec2{ 0.0f, 0.0f });

		ImGui::BeginChild("Toolbar");

		if (ImGui::Button("<-"))
		{
			AR_CONSOLE_LOG_INFO("Going up a directory");
		}

		ImGui::SameLine();

		if (ImGui::Button("->"))
		{
			AR_CONSOLE_LOG_INFO("Going in a directory");
		}

		ImGui::SameLine();

		if (ImGui::Button("R"))
		{
			AR_CONSOLE_LOG_INFO("Refreshing the Current Path");
		}

		ImGui::EndChild();
	}

	void ContentBrowserPanel::RenderBrowser(const ImVec2& size)
	{
	}

	void ContentBrowserPanel::Refresh()
	{
	}

	void ContentBrowserPanel::GoUpADirectory()
	{
	}

	void ContentBrowserPanel::GoInADirectory()
	{
	}

}