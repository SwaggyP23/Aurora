#include <Aurora.h>
#include <EntryPoint.h>

#include "EditorLayer.h"

namespace Aurora {

	class Luna : public Application
	{
	public:
		Luna(const ApplicationSpecification& spec, std::string_view projectPath)
			: Application(spec), m_ProjectPath(projectPath)
		{
		}

		virtual void OnInit() override
		{
			PushLayer(new EditorLayer());
		}

	private:
		std::string m_ProjectPath;

	};

	Application* CreateApplication(int argc, char** argv)
	{
		std::string projectPath = "";

		ApplicationSpecification specification;
		specification.Name = "Aurora";
		specification.WindowWidth = 1600;
		specification.WindowHeight = 900;
		specification.WindowDecorated = true;
		specification.StartMaximized = true;
		specification.VSync = true;

		specification.RendererConfiguration.EnvironmentMapResolution = 1024;
		specification.RendererConfiguration.IrradianceMapComputeSamples = 1024;

		return new Luna(specification, projectPath);
	}

}