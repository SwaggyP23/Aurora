#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/fmt/ostr.h>

#include <memory>

namespace Aurora {

	namespace logger {
		class Log
		{
		public:
			static void Init();

			static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
			static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }

		private:
			static std::shared_ptr<spdlog::logger> s_CoreLogger;
			static std::shared_ptr<spdlog::logger> s_ClientLogger;
		};
	}

}

#ifdef AURORA_DEBUG

	#define AR_CORE_LOG_TRACE(...)     ::Aurora::logger::Log::GetCoreLogger()->trace(__VA_ARGS__)
	#define AR_CORE_LOG_INFO(...)      ::Aurora::logger::Log::GetCoreLogger()->info(__VA_ARGS__)
	#define AR_CORE_LOG_WARN(...)      ::Aurora::logger::Log::GetCoreLogger()->warn(__VA_ARGS__)
	#define AR_CORE_LOG_ERROR(...)     ::Aurora::logger::Log::GetCoreLogger()->error(__VA_ARGS__)
	#define AR_CORE_LOG_CRITICAL(...)  ::Aurora::logger::Log::GetCoreLogger()->critical(__VA_ARGS__)

	#define AR_LOG_TRACE(...)          ::Aurora::logger::Log::GetClientLogger()->trace(__VA_ARGS__)
	#define AR_LOG_INFO(...)           ::Aurora::logger::Log::GetClientLogger()->info(__VA_ARGS__)
	#define AR_LOG_WARN(...)           ::Aurora::logger::Log::GetClientLogger()->warn(__VA_ARGS__)
	#define AR_LOG_ERROR(...)          ::Aurora::logger::Log::GetClientLogger()->error(__VA_ARGS__)
	#define AR_LOG_CRITICAL(...)       ::Aurora::logger::Log::GetCoreLogger()->critical(__VA_ARGS__)

#else

	#define AR_CORE_LOG_TRACE(...)
	#define AR_CORE_LOG_INFO(...)
	#define AR_CORE_LOG_WARN(...)
	#define AR_CORE_LOG_ERROR(...)
	#define AR_CORE_LOG_CRITICAL(...)

	#define AR_LOG_TRACE(...)
	#define AR_LOG_INFO(...)
	#define AR_LOG_WARN(...)
	#define AR_LOG_ERROR(...)
	#define AR_LOG_CRITICAL(...)

#endif