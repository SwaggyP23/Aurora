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

	#define AR_CORE_TRACE(...)     ::Aurora::Logger::Log::GetCoreLogger()->trace(__VA_ARGS__)
	#define AR_CORE_INFO(...)      ::Aurora::Logger::Log::GetCoreLogger()->info(__VA_ARGS__)
    #define AR_CORE_DEBUG(...)     ::Aurora::Logger::Log::GetCoreLogger()->debug(__VA_ARGS__)
	#define AR_CORE_WARN(...)      ::Aurora::Logger::Log::GetCoreLogger()->warn(__VA_ARGS__)
	#define AR_CORE_ERROR(...)     ::Aurora::Logger::Log::GetCoreLogger()->error(__VA_ARGS__)
	#define AR_CORE_CRITICAL(...)  ::Aurora::Logger::Log::GetCoreLogger()->critical(__VA_ARGS__)

	#define AR_TRACE(...)          ::Aurora::Logger::Log::GetClientLogger()->trace(__VA_ARGS__)
	#define AR_INFO(...)           ::Aurora::Logger::Log::GetClientLogger()->info(__VA_ARGS__)
    #define AR_DEBUG(...)          ::Aurora::Logger::Log::GetClientLogger()->debug(__VA_ARGS__)
	#define AR_WARN(...)           ::Aurora::Logger::Log::GetClientLogger()->warn(__VA_ARGS__)
	#define AR_ERROR(...)          ::Aurora::Logger::Log::GetClientLogger()->error(__VA_ARGS__)
	#define AR_CRITICAL(...)       ::Aurora::Logger::Log::GetCoreLogger()->critical(__VA_ARGS__)

#else

	#define AR_CORE_TRACE(...)
	#define AR_CORE_INFO(...)
    #define AR_CORE_DEBUG(...)
	#define AR_CORE_WARN(...)
	#define AR_CORE_ERROR(...)
	#define AR_CORE_CRITICAL(...)

	#define AR_TRACE(...)
	#define AR_INFO(...)
    #define AR_DEBUG(...)
	#define AR_WARN(...)
	#define AR_ERROR(...)
	#define AR_CRITICAL(...)

#endif