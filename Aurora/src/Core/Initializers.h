#pragma once

#include "Logging/Log.h"
#include "Utils/Random.h"

namespace Aurora {

	static void InitializeCore()
	{
		Logger::Log::Init();
		Utils::Random::Init();
	}

	static void ShutdownCore()
	{
		Logger::Log::ShutDown();
	}

}