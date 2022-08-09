#pragma once

#include "Logging/Log.h"
#include "Random.h"

namespace Aurora {

	static void InitializeCore()
	{
		Logger::Log::Init();
		Random::Init();
	}

	static void ShutdownCore()
	{
		Logger::Log::ShutDown();
	}

}