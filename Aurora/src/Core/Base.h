#pragma once

#include "Logging/Log.h"
#include "Refs.h"
#include <filesystem>
#include <stdint.h>

#ifndef AURORA_PLATFORM_WINDOWS
	#error Aurora only supports Windows for now!
#endif

#define AR_FORCE_INLINE __forceinline

#define AR_EXPAND_MACRO(x) x
#define AR_STRINGIFY_MACRO(x) #x

#define AR_PASTE_MACRO(x, y) x ## y
#define AR_CONCAT_MACRO(x, y) AR_PASTE_MACRO(x, y)

#define AR_DEBUG_BREAK() __debugbreak()

#include "Assert.h"

// Apparently using std::function and std::bind is not good and lambdas are way more efficient
#define AR_SET_EVENT_FN(function) [this](auto&&... args) -> decltype(auto) { return this->function(std::forward<decltype(args)>(args)...); }

#define BIT(x) 1 << x

namespace Aurora {

	template<typename T>
	using Scope = ScopedPointer<T>;

	template<typename T, typename... Args>
	constexpr Scope<T> CreateScope(Args&&... args)
	{
		return CreateScopedPointer<T>(std::forward<Args>(args)...);
	}

	template<typename T, typename... Args>
	constexpr Ref<T> CreateRef(Args&&... args)
	{
		return CreateReferencedObject<T>(std::forward<Args>(args)...);
	}

	using Byte = uint8_t;

}