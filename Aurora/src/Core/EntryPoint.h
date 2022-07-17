#pragma once

#include "Aurora.h"
#include "Application.h"

extern Aurora::Application* Aurora::CreateApplication();

int main(int argc, char** argv)
{
	Aurora::logger::Log::Init();

	AR_PROFILE_BEGIN_SESSION("ApplicationStartup", "Profiling");
	Aurora::Application* app = Aurora::CreateApplication();
	AR_PROFILE_END_SESSION("ApplicationStartup");

	AR_PROFILE_BEGIN_SESSION("ApplicationRuntime", "Profiling");
	app->Run();
	AR_PROFILE_END_SESSION("ApplicationRuntime");

	AR_PROFILE_BEGIN_SESSION("ApplicationShutdown", "Profiling");
	delete app;
	AR_PROFILE_END_SESSION("ApplicationShutdown");
}