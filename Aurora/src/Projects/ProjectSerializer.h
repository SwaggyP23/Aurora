#pragma once

#include "Project.h"

#include <filesystem>

namespace Aurora {

	class ProjectSerializer
	{
	public:
		static void Serialize(const std::filesystem::path& filepath, Ref<Project> project);
		static bool Deserialize(const std::filesystem::path& filepath, Ref<Project> project);

	};

}