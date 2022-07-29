#pragma once

#include "Logging/Log.h"
#include "Renderer/RendererPorperties.h"
#include "Utils/Random.h"

namespace Aurora {

	static void InitializeCore()
	{
		logger::Log::Init();
		Utils::Random::Init();
	}

	static void ShutdownCore()
	{
		logger::Log::ShutDown();
	}

}