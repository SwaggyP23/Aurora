#pragma once

#include "OGLpch.h"
#include "Application.h"

extern Application* CreateApplication(const std::string& name);

int main(int argc, char** argv)
{
	logger::Log::Init();

	PROFILE_BEGIN_SESSION("Startup", "Profiling/ApplicationStartup.json");
	Application* app = CreateApplication("Reda's App");
	PROFILE_END_SESSION();

	PROFILE_BEGIN_SESSION("Runtime", "Profiling/ApplicationRuntime.json");
	app->Run();
	PROFILE_END_SESSION();

	PROFILE_BEGIN_SESSION("Shutdown", "Profiling/ApplicationShutdown.json");
	delete app;
	PROFILE_END_SESSION();
}