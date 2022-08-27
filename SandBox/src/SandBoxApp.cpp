#include <Aurora.h>
#include <EntryPoint.h>
#include "SandBoxLayer.h"
#include "TestLayer.h"
#include "Snake/GameLayer.h"

class Sandbox : public Aurora::Application
{
public:
	Sandbox(const Aurora::ApplicationSpecification& spec)
		: Aurora::Application(spec)
	{
		//PushLayer(new SandBoxLayer());
		PushLayer(new TestLayer());
		//PushLayer(new GameLayer());
	}

};

Aurora::Application* Aurora::CreateApplication(int argc, char** argv)
{
	ApplicationSpecification specification;
	specification.Name = "Luna";
	specification.WindowWidth = 1600;
	specification.WindowHeight = 900;
	specification.VSync = true;

	return new Sandbox(specification);
}