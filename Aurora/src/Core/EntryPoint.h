#pragma once

#include "Aurora.h"
#include "Application.h"

extern Aurora::Application* Aurora::CreateApplication();

int main(int argc, char** argv)
{
	Aurora::logger::Log::Init();

	PROFILE_BEGIN_SESSION("Startup", "Profiling/ApplicationStartup.json");
	Aurora::Application* app = Aurora::CreateApplication();
	PROFILE_END_SESSION();

	PROFILE_BEGIN_SESSION("Runtime", "Profiling/ApplicationRuntime.json");
	app->Run();
	PROFILE_END_SESSION();

	PROFILE_BEGIN_SESSION("Shutdown", "Profiling/ApplicationShutdown.json");
	delete app;
	PROFILE_END_SESSION();
}