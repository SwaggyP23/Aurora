#include <OpenGLSeries.h>
#include "SandBoxLayer.h"
#include "TestLayer.h"
#include <Core/EntryPoint.h>

class Sandbox : public Application
{
public:
	Sandbox(const std::string& name)
		:	Application(name)
	{
		//pushLayer(new SandBoxLayer());
		pushLayer(new TestLayer());
	}

	~Sandbox()
	{
	}
};

Application* CreateApplication(const std::string& name)
{
	return new Sandbox(name);
}