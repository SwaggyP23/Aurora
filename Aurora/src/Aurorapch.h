#pragma once

#ifndef NOMINMAX
    #define NOMINMAX // These are for the random number generator
#endif // !NOMINMAX

#include <iostream>
#include <stdint.h>
#include <memory>
#include <utility>
#include <algorithm>
#include <functional>
#include <thread>
#include <variant>

#include <string>
#include <string_view>
#include <sstream>
#include <fstream>
#include <filesystem>

#include <array>
#include <vector>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <deque>

#include "Logging/Log.h"
#include "Core/Base.h"
#include "Utils/Utils.h"

#include "Debugging/Instrumentation.h"
#include "Debugging/Timer.h"

#include <Windows.h>