#pragma once

#include <memory>

#ifdef _DEBUG
	#define CORE_ASSERT(check, ...)  { if(!(check)) { CORE_LOG_ERROR("Assertion failed: {0}", __VA_ARGS__); __debugbreak(); }}
#else
	#define CORE_ASSERT(check, ...)
#endif

#define SET_EVENT_FN(x) std::bind(&x, this, std::placeholders::_1)

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