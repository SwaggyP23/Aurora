#pragma once

#include "OGLpch.h"
#include "Application.h"

extern Application* CreateApplication(const std::string& name);

int main()
{
	logger::Log::Init();

	Application* app = CreateApplication("Reda's App");
	app->Run();
	delete app;
}