#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>

#include <memory>

namespace Aurora {

	namespace Logger {
		class Log
		{
		public:
			static void Init();
			static void ShutDown();

			static const std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
			static const std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }

		private:
			static std::shared_ptr<spdlog::logger> s_CoreLogger;
			static std::shared_ptr<spdlog::logger> s_ClientLogger;
		};
	}

}

#ifndef AURORA_DIST

	#define AR_CORE_TRACE(...)     ::Aurora::logger::Log::GetCoreLogger()->trace(__VA_ARGS__)
	#define AR_CORE_INFO(...)      ::Aurora::logger::Log::GetCoreLogger()->info(__VA_ARGS__)
	#define AR_CORE_WARN(...)      ::Aurora::logger::Log::GetCoreLogger()->warn(__VA_ARGS__)
	#define AR_CORE_ERROR(...)     ::Aurora::logger::Log::GetCoreLogger()->error(__VA_ARGS__)
	#define AR_CORE_CRITICAL(...)  ::Aurora::logger::Log::GetCoreLogger()->critical(__VA_ARGS__)

	#define AR_TRACE(...)          ::Aurora::logger::Log::GetClientLogger()->trace(__VA_ARGS__)
	#define AR_INFO(...)           ::Aurora::logger::Log::GetClientLogger()->info(__VA_ARGS__)
	#define AR_WARN(...)           ::Aurora::logger::Log::GetClientLogger()->warn(__VA_ARGS__)
	#define AR_ERROR(...)          ::Aurora::logger::Log::GetClientLogger()->error(__VA_ARGS__)
	#define AR_CRITICAL(...)       ::Aurora::logger::Log::GetCoreLogger()->critical(__VA_ARGS__)

#else

	#define AR_CORE_TRACE(...)
	#define AR_CORE_INFO(...)
	#define AR_CORE_WARN(...)
	#define AR_CORE_ERROR(...)
	#define AR_CORE_CRITICAL(...)

	#define AR_TRACE(...)
	#define AR_INFO(...)
	#define AR_WARN(...)
	#define AR_ERROR(...)
	#define AR_CRITICAL(...)

#endif