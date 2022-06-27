#include <OpenGLSeries.h>
#include <Core/EntryPoint.h>

class Sandbox : public Application
{
public:
	Sandbox()
	{
		// PushLayer(new ExampleLayer());
	}

	~Sandbox()
	{
	}
};

Application* CreateApplication()
{
	return new Sandbox();
}