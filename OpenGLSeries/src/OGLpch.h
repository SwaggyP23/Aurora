#pragma once

#define SET_EVENT_FN(x) std::bind(&x, this, std::placeholders::_1)

#include <Windows.h>

#include <iostream>
#include <stdint.h>
#include <memory>
#include <utility>
#include <algorithm>
#include <functional>

#include <string>
#include <sstream>
#include <fstream>
#include <array>
#include <vector>

#include <stb_image/stb_image.h>

#include "Logging/Log.h"
#include "Utils/ImageLoader.h"
#include "Utils/UtilFunctions.h"