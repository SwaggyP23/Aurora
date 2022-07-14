#pragma once

#ifndef NOMINMAX
#define NOMINMAX // This are for the random number generator
#endif // !NOMINMAX

#include <iostream>
#include <stdint.h>
#include <memory>
#include <utility>
#include <algorithm>
#include <functional>
#include <thread>

#include <string>
#include <sstream>
#include <fstream>
#include <array>
#include <vector>
#include <map>
#include <unordered_map>
#include <unordered_set>

#include <stb_image/stb_image.h>

#include "Utils/Random.h"
#include "Logging/Log.h"
#include "Core/Base.h"
#include "Utils/ImageLoader.h"
#include "Utils/UtilFunctions.h"
#include "Debugging/Instrumentor.h"
#include "Debugging/Timer.h"

#include <Windows.h>