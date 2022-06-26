#include "Application.h"

int main()
{
	Application* app = new Application("OpenGL App");
	app->Run();
	delete app;
}