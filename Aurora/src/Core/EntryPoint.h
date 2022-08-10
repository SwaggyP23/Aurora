#pragma once

#include "Initializers.h"

#ifdef AR_PLATFORM_WINDOWS

extern Aurora::Application* Aurora::CreateApplication(int argc, char** argv);
bool g_ApplicationRunning = true;

namespace Aurora {

	int Main(int argc, char** argv)
	{
		while (g_ApplicationRunning)
		{
			Aurora::InitializeCore();
			Aurora::Application* app = Aurora::CreateApplication(argc, argv);			
			AR_CORE_ASSERT(app, "Main", "Application is null!");
			app->Run();
			delete app;
			Aurora::ShutdownCore();
		}

		return 0;
	}

}

#ifdef AURORA_DIST

int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow)
{
	return Aurora::Main(__argc, __argv);
}

#else

int main(int argc, char** argv)
{
	return Aurora::Main(argc, argv);
}

#endif

#endif