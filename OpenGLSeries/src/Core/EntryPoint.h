#pragma once

#include "OGLpch.h"
#include "Application.h"

int main()
{
	logger::Log::Init();

	Application* app = new Application("OpenGL App");
	app->Run();
	delete app;
}