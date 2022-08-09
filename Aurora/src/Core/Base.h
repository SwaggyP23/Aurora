#pragma once

#include "Logging/Log.h"
#include "Refs.h"
#include <filesystem>

#define AR_EXPAND_MACRO(x) x
#define AR_STRINGIFY_MACRO(x) #x

#define AR_PASTE_MACRO(x, y) x ## y
#define AR_CONCAT_MACRO(x, y) AR_PASTE_MACRO(x, y)


#ifdef AURORA_DEBUG
	#define AR_DEBUG_BREAK __debugbreak()
    #define AR_CORE_ASSERT(check, ...)  { if(!(check)) { AR_CORE_ERROR("Assertion '{0}' failed at: {1}:{2}\n\tMessage: {3}", AR_STRINGIFY_MACRO(check), std::filesystem::path(__FILE__).filename().string(), __LINE__, __VA_ARGS__); AR_DEBUG_BREAK; }}
#else
    #define AR_CORE_ASSERT(check, ...)
#endif

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

	template<typename T>
	using Ref = RefCountedObject<T>;

	template<typename T, typename... Args>
	constexpr Ref<T> CreateRef(Args&&... args)
	{
		return CreateReferencedObject<T>(std::forward<Args>(args)...);
	}

}