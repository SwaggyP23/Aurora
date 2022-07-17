#pragma once

#include <memory>

#ifdef AURORA_DEBUG
	#define AR_CORE_ASSERT(check, ...)  { if(!(check)) { AR_CORE_ERROR("Assertion failed: {0}", __VA_ARGS__); __debugbreak(); }}
#else
	#define AR_CORE_ASSERT(check, ...)
#endif

#define STRINGIFY(A, B, C) A B C // Concatenates three strings. This is to be used for strings and STRINGS ONLY!!

#define AR_SET_EVENT_FN(x) std::bind(&x, this, std::placeholders::_1)

#define BIT(x) 1 << x

namespace Aurora {

	template<typename T>
	using Scope = std::unique_ptr<T>;

	template<typename T, typename... Args>
	constexpr Scope<T> CreateScope(Args&&... args)
	{
		return std::make_unique<T>(std::forward<Args>(args)...);
	}

	template<typename T>
	using Ref = std::shared_ptr<T>;

	template<typename T, typename... Args>
	constexpr Ref<T> CreateRef(Args&&... args)
	{
		return std::make_shared<T>(std::forward<Args>(args)...);
	}

}