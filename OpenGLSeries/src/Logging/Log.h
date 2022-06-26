#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/fmt/ostr.h>

#include <memory>

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

#if _DEBUG

#define CORE_LOG_TRACE(...)     logger::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define CORE_LOG_INFO(...)      logger::Log::GetCoreLogger()->info(__VA_ARGS__)
#define CORE_LOG_WARN(...)      logger::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define CORE_LOG_ERROR(...)     logger::Log::GetCoreLogger()->error(__VA_ARGS__)

#define LOG_TRACE(...)          logger::Log::GetClientLogger()->trace(__VA_ARGS__)
#define LOG_INFO(...)           logger::Log::GetClientLogger()->info(__VA_ARGS__)
#define LOG_WARN(...)           logger::Log::GetClientLogger()->warn(__VA_ARGS__)
#define LOG_ERROR(...)          logger::Log::GetClientLogger()->error(__VA_ARGS__)

#else

#define CORE_LOG_TRACE(...)
#define CORE_LOG_INFO(...)
#define CORE_LOG_WARN(...)
#define CORE_LOG_ERROR(...)

#define LOG_TRACE(...)
#define LOG_INFO(...)
#define LOG_WARN(...)
#define LOG_ERROR(...)

#endif