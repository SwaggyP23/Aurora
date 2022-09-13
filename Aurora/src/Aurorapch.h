#pragma once

#ifdef AURORA_PLATFORM_WINDOWS
    #ifndef NOMINMAX
        #define NOMINMAX // These are for the random number generator
    #endif // !NOMINMAX
#endif

#include <memory>
#include <utility>
#include <algorithm>
#include <functional>
#include <thread>

#include <string>
#include <string_view>
#include <sstream>
#include <fstream>
#include <filesystem>

#include <array>
#include <vector>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>

#include "Logging/Log.h"
#include "Core/Base.h"

#include "Debugging/Instrumentation.h"
#include "Debugging/Timer.h"

#ifdef AURORA_PLATFORM_WINDOWS
    #include <Windows.h>
#endif