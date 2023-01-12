#include <Aurora.h>
#include <EntryPoint.h>

#include "RuntimeLayer.h"

class RuntimeApplication : public Aurora::Application
{
public:
	// TODO: Switch to projectPath once projects are a thing
	RuntimeApplication(const Aurora::ApplicationSpecification& spec, std::string_view scenePath)
		: Aurora::Application(spec), m_ScenePath(scenePath)
	{
	}

	virtual void OnInit() override
	{
		PushLayer(new Aurora::RuntimeLayer(m_ScenePath));
	}

private:
	std::string m_ScenePath; // TODO: Switch to projectPath once projects are a thing

};

Aurora::Application* Aurora::CreateApplication(int argc, char** argv)
{
	std::string scenePath = "Resources/scenes/StaticMeshTest.ascene"; // TODO: Set scene in SandboxProject Directory!

	ApplicationSpecification specification;
	specification.Name = "Aurora Runtime";
	specification.WindowWidth = 1280;
	specification.WindowHeight = 720;
	specification.WindowDecorated = true;
	specification.Fullscreen = true;
	specification.StartMaximized = false;
	specification.VSync = true;
	specification.EnableImGui = false;

	// IMPORTANT: Disable for actual DIST builds? tf
	specification.WorkingDirectory = "../Luna";

	return new RuntimeApplication(specification, scenePath);
}