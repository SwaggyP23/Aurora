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
		specification.Name = "Luna";
		specification.WindowWidth = 1600;
		specification.WindowHeight = 900;
		specification.WindowDecorated = true;
		specification.StartMaximized = true;
		specification.VSync = true;
		specification.ApplicationWindowIconPath = "../Resources/Icons/AuroraIcon1.png";

		//specification.WorkingDirectory // TODO: Needs to be handled...

		specification.RendererConfiguration.EnvironmentMapResolution = 1024;
		specification.RendererConfiguration.IrradianceMapComputeSamples = 1024;

		return new Luna(specification);
	}

}