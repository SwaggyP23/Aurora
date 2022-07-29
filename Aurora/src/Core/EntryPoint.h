#pragma once

#include "Initializers.h"
#include "Application.h"

extern Aurora::Application* Aurora::CreateApplication(int argc, char** argv);
bool g_ApplicationRunning = true;

int main(int argc, char** argv)
{
	while (g_ApplicationRunning)
	{
		Aurora::InitializeCore();

		AR_PROFILE_BEGIN_SESSION("ApplicationStartup", "Profiling");
		Aurora::Application* app = Aurora::CreateApplication(argc, argv);
		AR_PROFILE_END_SESSION("ApplicationStartup");

		AR_CORE_ASSERT(app, "Application is null!");

		AR_PROFILE_BEGIN_SESSION("ApplicationRuntime", "Profiling");
		app->Run();
		AR_PROFILE_END_SESSION("ApplicationRuntime");

		AR_PROFILE_BEGIN_SESSION("ApplicationShutdown", "Profiling");
		delete app;
		AR_PROFILE_END_SESSION("ApplicationShutdown");

		Aurora::ShutdownCore();
	}

	return 0;
}