#pragma once

#include "Core/Base.h"

#include <filesystem>

namespace Aurora {

	struct ProjectSpecification
	{
		std::string Name;
		
		std::string AssetsDirectory;
		std::string AssetsRegistryPath;

		// MeshPath??
		// MeshSourcePath?? but whyy???

		// TODO: Add script module path

		std::string StartScene;

		// Not serialized
		std::string ProjectFileName;
		std::string ProjectDirectory;
	};

	class Project : public RefCountedObject
	{
	public:
		Project();
		~Project();

		static Ref<Project> Create();

		const ProjectSpecification& GetSpec() const { return m_Specification; }

		static void SetActiveProject(Ref<Project> project);
		static Ref<Project> GetActiveProject() { return s_ActiveProject; }

		static const std::string& GetProjectName()
		{
			AR_CORE_ASSERT(s_ActiveProject);
			return s_ActiveProject->GetSpec().Name;
		}

		static std::filesystem::path GetProjectDirectoy()
		{
			AR_CORE_ASSERT(s_ActiveProject);
			return s_ActiveProject->GetSpec().ProjectDirectory;
		}

		static std::filesystem::path GetAssetsDirectory()
		{
			AR_CORE_ASSERT(s_ActiveProject);
			return std::filesystem::path(s_ActiveProject->GetSpec().ProjectDirectory) / s_ActiveProject->GetSpec().AssetsDirectory;
		}

		static std::filesystem::path GetAssetsRegistryPath()
		{
			AR_CORE_ASSERT(s_ActiveProject);
			return std::filesystem::path(s_ActiveProject->GetSpec().ProjectDirectory) / s_ActiveProject->GetSpec().AssetsRegistryPath;
		}

		// This is for the future in case we need to cache anything from the engine systems, However the internal engine cache like shaders, etc...
		// These are cached in a separate cache folder inside resources
		static std::filesystem::path GetCacheDirectory()
		{
			AR_CORE_ASSERT(s_ActiveProject);
			return std::filesystem::path(s_ActiveProject->GetSpec().ProjectDirectory) / "Cache";
		}

	private:
		ProjectSpecification m_Specification;

		inline static Ref<Project> s_ActiveProject;

		friend class ProjectSerializer;
		friend class ProjectSettingsPanel; // TODO: ...

	};

}