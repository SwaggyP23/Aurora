#include <Aurora.h>
#include "SandBoxLayer.h"
#include "TestLayer.h"
#include "Snake/GameLayer.h"
#include <Core/EntryPoint.h>

class Sandbox : public Aurora::Application
{
public:
	Sandbox()
		: Aurora::Application("SandBox App")
	{
		//pushLayer(new SandBoxLayer());
		pushLayer(new TestLayer());
		//pushLayer(new GameLayer());
	}

	~Sandbox()
	{
	}
};

Aurora::Application* Aurora::CreateApplication()
{
	return new Sandbox();
}