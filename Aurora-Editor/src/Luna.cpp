#include <Aurora.h>
#include <EntryPoint.h>

#include "EditorLayer.h"

namespace Aurora {

	class Luna : public Application
	{
	public:
		Luna(const ApplicationSpecification& spec)
			: Application(spec)
		{
		}

		virtual void OnInit() override
		{
			PushLayer(new EditorLayer());
		}

	};

	Application* CreateApplication(int argc, char** argv)
	{
		ApplicationSpecification specification;
		specification.Name = "Aurora";
		specification.WindowWidth = 1600;
		specification.WindowHeight = 900;
		specification.WindowDecorated = true;
		specification.StartMaximized = true;
		specification.VSync = true;

		specification.RendererConfiguration.EnvironmentMapResolution = 1024;
		specification.RendererConfiguration.IrradianceMapComputeSamples = 1024;

		return new Luna(specification);
	}

}