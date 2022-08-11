#pragma once

#include "Logging/Log.h"
#include "Random.h"

namespace Aurora {

	static void InitializeCore()
	{
		Logger::Log::Init();
		Random::Init();

		AR_CORE_TRACE_TAG("Core", "Aurora Engine");
		AR_CORE_TRACE_TAG("Core", "Initializing...");
	}

	static void ShutdownCore()
	{
		AR_CORE_TRACE_TAG("Core", "Shutting down...");

		Logger::Log::ShutDown();
	}

}