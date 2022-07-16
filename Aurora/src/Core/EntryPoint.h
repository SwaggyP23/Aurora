#pragma once

#include "Aurora.h"
#include "Application.h"

extern Aurora::Application* Aurora::CreateApplication();

int main(int argc, char** argv)
{
	Aurora::logger::Log::Init();

	AR_OP_START_CAPTURE();
	Aurora::Application* app = Aurora::CreateApplication();
	AR_OP_STOP_CAPTURE("Profiling/ApplicationStartup.opt");

	AR_OP_START_CAPTURE();
	app->Run();
	AR_OP_STOP_CAPTURE("Profiling/ApplicationRuntime.opt");

	AR_OP_START_CAPTURE();
	delete app;
	AR_OP_STOP_CAPTURE("Profiling/ApplicationShutdown.opt");
}