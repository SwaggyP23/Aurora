#include "Aurorapch.h"
#include "ProjectSerializer.h"

#include <yaml-cpp/yaml.h>

namespace Aurora {

	void ProjectSerializer::Serialize(const std::filesystem::path& filepath, Ref<Project> project)
	{
		YAML::Emitter out;

		out << YAML::BeginMap;

		out << YAML::Key << "Project";

		{
			out << YAML::BeginMap;

			// TODO: Project stuff...

			out << YAML::Key << "Log";
			{
				out << YAML::BeginMap;

				const std::map<std::string, Logger::Log::TagDetails>& tags = Logger::Log::EnabledTags();
				for (const auto& [tag, detail] : tags)
				{
					if (!tag.empty()) // Un Tagged loggers are not serialized!
					{
						out << YAML::Key << tag;
						out << YAML::BeginMap;

						out << YAML::Key << "Enabled" << YAML::Value << detail.Enabled;
						out << YAML::Key << "LevelFilter" << YAML::Value << Logger::Log::LevelToString(detail.LevelFilter);

						out << YAML::EndMap;
					}
				}

				out << YAML::EndMap;
			}

			out << YAML::EndMap;
		}

		out << YAML::EndMap;

		std::ofstream ofStream(filepath); // RAII releases the handle
		ofStream << out.c_str();
	}

	bool ProjectSerializer::Deserialize(const std::filesystem::path& filepath, Ref<Project> project)
	{
		std::ifstream ifStream(filepath);
		AR_CORE_ASSERT(ifStream);
		std::stringstream strStream;
		strStream << ifStream.rdbuf();

		YAML::Node data = YAML::Load(strStream.str());
		if (!data["Project"])
		{
			return false;
		}

		YAML::Node rootNode = data["Project"];
		//if (!rootNode["Name"]) // TODO: After adding in Serialize...
		//{
		//	return false;
		//}

		ProjectSpecification& spec = project->m_Specification;

		// Log...
		YAML::Node logNode = rootNode["Log"];
		std::map<std::string, Logger::Log::TagDetails>& tags = Logger::Log::EnabledTags();
		for (auto node : logNode)
		{
			std::string name = node.first.as<std::string>();
			Logger::Log::TagDetails& details = tags[name];
			details.Enabled = node.second["Enabled"].as<bool>();
			details.LevelFilter = Logger::Log::LevelFromString(node.second["LevelFilter"].as<std::string>());
		}

		return true;
	}

}