#include "Aurorapch.h"
#include "Project.h"

#include "AssetManager/AssetManager.h"

namespace Aurora {

	Ref<Project> Project::Create()
	{
		return CreateRef<Project>();
	}

	Project::Project()
	{
	}

	Project::~Project()
	{
	}

	void Project::SetActiveProject(Ref<Project> project)
	{
		if (s_ActiveProject)
		{
			AssetManager::Shutdown();
		}

		s_ActiveProject = project;
		if (s_ActiveProject)
		{
			AssetManager::Init();
		}
	}

}