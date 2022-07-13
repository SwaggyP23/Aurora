#pragma once

#include "Aurora.h"
#include "Application.h"

extern Aurora::Application* Aurora::CreateApplication();

int main(int argc, char** argv)
{
	Aurora::logger::Log::Init();

	AR_PROFILE_BEGIN_SESSION("Startup", "Profiling/ApplicationStartup.json");
	Aurora::Application* app = Aurora::CreateApplication();
	AR_PROFILE_END_SESSION();

	AR_PROFILE_BEGIN_SESSION("Runtime", "Profiling/ApplicationRuntime.json");
	app->Run();
	AR_PROFILE_END_SESSION();

	AR_PROFILE_BEGIN_SESSION("Shutdown", "Profiling/ApplicationShutdown.json");
	delete app;
	AR_PROFILE_END_SESSION();
}