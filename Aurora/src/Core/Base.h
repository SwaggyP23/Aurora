#pragma once

#include "Refs.h"
#include "Logging/Log.h"

#ifdef AURORA_DEBUG
	#define AR_CORE_ASSERT(check, ...)  { if(!(check)) { AR_CORE_ERROR("Assertion failed: {0}", __VA_ARGS__); __debugbreak(); }}
#else
	#define AR_CORE_ASSERT(check, ...)
#endif

#define AR_EXPAND_MACRO(x) x
#define AR_STRINGIFY_MACRO(x) #x

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