#include "Aurorapch.h"
#include "EditorPanelLibrary.h"

#include "EditorStylePanel.h"
#include "ApplicationSettingsPanel.h"

#include <yaml-cpp/yaml.h>

namespace Aurora {

	Scope<EditorPanelsLibrary> EditorPanelsLibrary::Create()
	{
		return CreateScope<EditorPanelsLibrary>();
	}

	EditorPanelsLibrary::~EditorPanelsLibrary()
	{
		for (auto& panelMap : m_Panels)
			panelMap.clear();
	}

	void EditorPanelsLibrary::OnImGuiRender()
	{
		for (auto& panelMap : m_Panels)
		{
			for (auto& [id, panelSpec] : panelMap)
			{
				bool stateChanged = false;

				if (panelSpec.IsOpen)
				{
					panelSpec.Panel->OnImGuiRender(panelSpec.IsOpen);
					stateChanged = !panelSpec.IsOpen;
				}

				if (stateChanged)
					Serialize();
			}
		}
	}

	void EditorPanelsLibrary::OnEvent(Event& e)
	{
		for (auto& panelMap : m_Panels)
		{
			for (auto& [id, panelSpec] : panelMap)
				panelSpec.Panel->OnEvent(e);
		}
	}

	void EditorPanelsLibrary::OnProjectChanged(Ref<Project> project)
	{
		for (auto& panelMap : m_Panels)
		{
			for (auto& [id, panelSpec] : panelMap)
			{
				panelSpec.Panel->OnProjectChanged(project);
			}
		}

		Deserialize();
	}

	void EditorPanelsLibrary::SetSceneContext(Ref<Scene> scene)
	{
		for (auto& panelMap : m_Panels)
		{
			for (auto& [id, panelSpec] : panelMap)
				panelSpec.Panel->SetSceneContext(scene);
		}
	}

	void EditorPanelsLibrary::Serialize()
	{
		YAML::Emitter out;

		out << YAML::BeginMap;

		out << YAML::Key << "Panels" << YAML::BeginSeq;
		for (size_t category = 0; category < m_Panels.size(); category++)
		{
			for (auto& [id, panelSpec] : m_Panels[category])
			{
				out << YAML::BeginMap;

				out << YAML::Key << "ID" << YAML::Value << id;
				out << YAML::Key << "Name" << YAML::Value << panelSpec.Name;
				out << YAML::Key << "IsOpen" << YAML::Value << panelSpec.IsOpen;

				out << YAML::EndMap;
			}
		}
		out << YAML::EndSeq;

		EditorStylePanel::Serialize(out);
		ApplicationSettingsPanel::Serialize(out);

		out << YAML::EndMap;

		std::ofstream ofStream("Config/EditorSettings.aeditor");
		ofStream << out.c_str();
	}

	void EditorPanelsLibrary::Deserialize()
	{
		std::filesystem::path path = "Config/EditorSettings.aeditor";
		if (!std::filesystem::exists(path))
			return;

		std::ifstream ifStream(path);
		AR_CORE_CHECK(ifStream);
		std::stringstream strStream;

		strStream << ifStream.rdbuf();

		YAML::Node data = YAML::Load(strStream.str());
		if (!data["Panels"])
		{
			AR_CONSOLE_LOG_ERROR("[EditorPanelsLibrary]: Failed to load EditorSettings.aeditor!");
			return;
		}

		for (auto panelNode : data["Panels"])
		{
			PanelSpecification* spec = GetPanelSpec(panelNode["ID"].as<uint32_t>(0));

			if (spec == nullptr)
				continue;

			// Dont need to deserialize the name since it will be created by the panels themselves!
			spec->IsOpen = panelNode["IsOpen"].as<bool>(spec->IsOpen);
		}

		EditorStylePanel::Deserialize(data);
		ApplicationSettingsPanel::Deserialize(data);
	}

	void EditorPanelsLibrary::RemovePanel(const char* strId)
	{
		uint32_t panelhash = Hash::GenerateFNVHash(strId);

		for (auto& panelMap : m_Panels)
		{
			if (panelMap.find(panelhash) != panelMap.end())
			{
				panelMap.erase(panelhash);

				return;
			}
		}

		AR_CONSOLE_LOG_ERROR("[EditorPanelsLibrary] Did not find panel with str ID {0} for removing", strId);
	}

	PanelSpecification* EditorPanelsLibrary::GetPanelSpec(uint32_t id)
	{
		for (auto& panelMap : m_Panels)
		{
			if (panelMap.find(id) != panelMap.end())
				return &panelMap.at(id);
		}

		return nullptr;
	}

	const PanelSpecification* EditorPanelsLibrary::GetPanelSpec(uint32_t id) const
	{
		for (const auto& panelMap : m_Panels)
		{
			if (panelMap.find(id) != panelMap.end())
				return &panelMap.at(id);
		}

		return nullptr;
	}

}