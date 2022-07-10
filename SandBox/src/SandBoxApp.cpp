#include <Aurora.h>
#include "SandBoxLayer.h"
#include "TestLayer.h"
#include "Snake/GameLayer.h"
#include <Core/EntryPoint.h>

class Sandbox : public Aurora::Application
{
public:
	Sandbox(const std::string& name)
		: Aurora::Application(name)
	{
		//pushLayer(new SandBoxLayer());
		pushLayer(new TestLayer());
		//pushLayer(new GameLayer());
	}

	~Sandbox()
	{
	}
};

Aurora::Application* Aurora::CreateApplication(const std::string& name)
{
	return new Sandbox(name);
}