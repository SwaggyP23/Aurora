#include "ContentBrowserPanel.h"

namespace Aurora {

	Ref<ContentBrowserPanel> ContentBrowserPanel::Create()
	{
		return CreateRef<ContentBrowserPanel>();
	}

	void ContentBrowserPanel::OnImGuiRender(bool& isOpen)
	{
		ImGui::Begin("Content Browser", &isOpen);

		ImGui::Text("9uvanreouivnhun");

		ImGui::End();
	}

}