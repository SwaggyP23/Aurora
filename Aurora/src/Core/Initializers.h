#pragma once

#include "Base.h"
#include "Random.h"

namespace Aurora {

	static void InitializeCore()
	{
		Logger::Log::Init();
		Random::Init();

		AR_CORE_INFO_TAG("Core", "Aurora Engine");
		AR_CORE_INFO_TAG("Core", "Initializing...");
	}

	static void ShutdownCore()
	{
		AR_CORE_INFO_TAG("Core", "Shutting down...");

		Logger::Log::ShutDown();
	}

}