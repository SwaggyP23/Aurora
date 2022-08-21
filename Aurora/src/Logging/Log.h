#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>

#include <map>
#include <memory>

namespace Aurora { namespace Logger {

	class Log
	{
	public:
		enum class Type : uint8_t
		{
			Core = 0, Client = 1
		};

		enum class Level : uint8_t
		{
			Trace = 0, Info, Debug, Warn, Error, Fatal
		};

		struct TagDetails
		{
			bool Enabled = true;
			Level LevelFilter = Level::Trace;
		};

		static void Init();
		static void ShutDown();

		static const std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
		static const std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }

		static bool HasTag(const std::string& tag) { return s_EnabledTags.find(tag) != s_EnabledTags.end(); }
		static std::map<std::string, TagDetails>& EnabledTags() { return s_EnabledTags; }

		template<typename... Args>
		static void PrintMessage(Log::Type type, Log::Level level, std::string_view tag, Args&&... args);

		template<typename... Args>
		static void PrintAssertMessageWithTag(Log::Type type, std::string_view tag, std::string_view assertFailed, Args&&... args);

	public:
		// Enum utils
		static const char* LevelToString(Level level)
		{
			switch (level)
			{
			    case Level::Trace: return "Trace";
			    case Level::Info:  return "Info";
			    case Level::Debug: return "Debug";
			    case Level::Warn:  return "Warn";
			    case Level::Error: return "Error";
			    case Level::Fatal: return "Fatal";
			}

			return "";
		}
		static Level LevelFromString(std::string_view string)
		{
			if (string == "Trace") return Level::Trace;
			if (string == "Info")  return Level::Info;
			if (string == "Debug") return Level::Debug;
			if (string == "Warn")  return Level::Warn;
			if (string == "Error") return Level::Error;
			if (string == "Fatal") return Level::Fatal;

			return Level::Trace;
		}

	private:
		static std::shared_ptr<spdlog::logger> s_CoreLogger;
		static std::shared_ptr<spdlog::logger> s_ClientLogger;

		inline static std::map<std::string, TagDetails> s_EnabledTags;

	};

	template<typename... Args>
	void Log::PrintMessage(Log::Type type, Log::Level level, std::string_view tag, Args&&... args)
	{
		const auto& detail = s_EnabledTags[std::string(tag)];
		if (detail.Enabled && detail.LevelFilter <= level)
		{
			const auto& logger = (type == Type::Core) ? GetCoreLogger() : GetClientLogger();
			std::string logString = tag.empty() ? "{0}{1}" : "[{0}]: {1}";
			switch (level)
			{
			case Level::Trace:
				logger->trace(logString, tag, fmt::format(std::forward<Args>(args)...));
				break;
			case Level::Info:
				logger->info(logString, tag, fmt::format(std::forward<Args>(args)...));
				break;
			case Level::Debug:
				logger->debug(logString, tag, fmt::format(std::forward<Args>(args)...));
				break;
			case Level::Warn:
				logger->warn(logString, tag, fmt::format(std::forward<Args>(args)...));
				break;
			case Level::Error:
				logger->error(logString, tag, fmt::format(std::forward<Args>(args)...));
				break;
			case Level::Fatal:
				logger->critical(logString, tag, fmt::format(std::forward<Args>(args)...));
				break;
			}
		}
	}

	template<typename... Args>
	void Log::PrintAssertMessageWithTag(Log::Type type, std::string_view tag, std::string_view assertFailed, Args&&... args)
	{
		const auto& logger = (type == Type::Core) ? GetCoreLogger() : GetClientLogger();
		logger->error("[{0}]: {1}\n\t\t\t  Error: {2}", tag, assertFailed, fmt::format(std::forward<Args>(args)...));
	}

	// Without this template specializtion, assert without messages are not possible since the function would still want more arguments for the variadic template!
	template<>
	inline void Log::PrintAssertMessageWithTag(Log::Type type, std::string_view tag, std::string_view assertFailed)
	{
		const auto& logger = (type == Type::Core) ? GetCoreLogger() : GetClientLogger();
		logger->error("[{0}]: {1}", tag, assertFailed);
	}

} }

#ifndef AURORA_DIST

    #define AR_CORE_TRACE_TAG(tag, ...)     ::Aurora::Logger::Log::PrintMessage(::Aurora::Logger::Log::Type::Core, ::Aurora::Logger::Log::Level::Trace, tag, __VA_ARGS__)
    #define AR_CORE_INFO_TAG(tag, ...)      ::Aurora::Logger::Log::PrintMessage(::Aurora::Logger::Log::Type::Core, ::Aurora::Logger::Log::Level::Info, tag, __VA_ARGS__)
    #define AR_CORE_DEBUG_TAG(tag, ...)     ::Aurora::Logger::Log::PrintMessage(::Aurora::Logger::Log::Type::Core, ::Aurora::Logger::Log::Level::Debug, tag, __VA_ARGS__)
    #define AR_CORE_WARN_TAG(tag, ...)      ::Aurora::Logger::Log::PrintMessage(::Aurora::Logger::Log::Type::Core, ::Aurora::Logger::Log::Level::Warn, tag, __VA_ARGS__)
    #define AR_CORE_ERROR_TAG(tag, ...)     ::Aurora::Logger::Log::PrintMessage(::Aurora::Logger::Log::Type::Core, ::Aurora::Logger::Log::Level::Error, tag, __VA_ARGS__)
    #define AR_CORE_CRITICAL_TAG(tag, ...)  ::Aurora::Logger::Log::PrintMessage(::Aurora::Logger::Log::Type::Core, ::Aurora::Logger::Log::Level::Fatal, tag, __VA_ARGS__)

	#define AR_CORE_TRACE(...)              ::Aurora::Logger::Log::PrintMessage(::Aurora::Logger::Log::Type::Core, ::Aurora::Logger::Log::Level::Trace, "", __VA_ARGS__)
    #define AR_CORE_INFO(...)               ::Aurora::Logger::Log::PrintMessage(::Aurora::Logger::Log::Type::Core, ::Aurora::Logger::Log::Level::Info, "", __VA_ARGS__)
    #define AR_CORE_DEBUG(...)              ::Aurora::Logger::Log::PrintMessage(::Aurora::Logger::Log::Type::Core, ::Aurora::Logger::Log::Level::Debug, "", __VA_ARGS__)
    #define AR_CORE_WARN(...)               ::Aurora::Logger::Log::PrintMessage(::Aurora::Logger::Log::Type::Core, ::Aurora::Logger::Log::Level::Warn, "", __VA_ARGS__)
    #define AR_CORE_ERROR(...)              ::Aurora::Logger::Log::PrintMessage(::Aurora::Logger::Log::Type::Core, ::Aurora::Logger::Log::Level::Error, "", __VA_ARGS__)
    #define AR_CORE_CRITICAL(...)           ::Aurora::Logger::Log::PrintMessage(::Aurora::Logger::Log::Type::Core, ::Aurora::Logger::Log::Level::Fatal, "", __VA_ARGS__)

	#define AR_TRACE_TAG(tag, ...)          ::Aurora::Logger::Log::PrintMessage(::Aurora::Logger::Log::Type::Client, ::Aurora::Logger::Log::Level::Trace, tag, __VA_ARGS__)
    #define AR_INFO_TAG(tag, ...)           ::Aurora::Logger::Log::PrintMessage(::Aurora::Logger::Log::Type::Client, ::Aurora::Logger::Log::Level::Info, tag, __VA_ARGS__)
    #define AR_DEBUG_TAG(tag, ...)          ::Aurora::Logger::Log::PrintMessage(::Aurora::Logger::Log::Type::Client, ::Aurora::Logger::Log::Level::Debug, tag, __VA_ARGS__)
    #define AR_WARN_TAG(tag, ...)           ::Aurora::Logger::Log::PrintMessage(::Aurora::Logger::Log::Type::Client, ::Aurora::Logger::Log::Level::Warn, tag, __VA_ARGS__)
    #define AR_ERROR_TAG(tag, ...)          ::Aurora::Logger::Log::PrintMessage(::Aurora::Logger::Log::Type::Client, ::Aurora::Logger::Log::Level::Error, tag, __VA_ARGS__)
    #define AR_CRITICAL_TAG(tag, ...)       ::Aurora::Logger::Log::PrintMessage(::Aurora::Logger::Log::Type::Client, ::Aurora::Logger::Log::Level::Fatal, tag, __VA_ARGS__)

    #define AR_TRACE(...)                   ::Aurora::Logger::Log::PrintMessage(::Aurora::Logger::Log::Type::Client, ::Aurora::Logger::Log::Level::Trace, "", __VA_ARGS__)
    #define AR_INFO(...)                    ::Aurora::Logger::Log::PrintMessage(::Aurora::Logger::Log::Type::Client, ::Aurora::Logger::Log::Level::Info, "", __VA_ARGS__)
    #define AR_DEBUG(...)                   ::Aurora::Logger::Log::PrintMessage(::Aurora::Logger::Log::Type::Client, ::Aurora::Logger::Log::Level::Debug, "", __VA_ARGS__)
    #define AR_WARN(...)                    ::Aurora::Logger::Log::PrintMessage(::Aurora::Logger::Log::Type::Client, ::Aurora::Logger::Log::Level::Warn, "", __VA_ARGS__)
    #define AR_ERROR(...)                   ::Aurora::Logger::Log::PrintMessage(::Aurora::Logger::Log::Type::Client, ::Aurora::Logger::Log::Level::Error, "", __VA_ARGS__)
    #define AR_CRITICAL(...)                ::Aurora::Logger::Log::PrintMessage(::Aurora::Logger::Log::Type::Client, ::Aurora::Logger::Log::Level::Fatal, "", __VA_ARGS__)

#else

    #define AR_CORE_TRACE_TAG(tag, ...)
    #define AR_CORE_INFO_TAG(tag, ...)
    #define AR_CORE_DEBUG_TAG(tag, ...)
    #define AR_CORE_WARN_TAG(tag, ...)
    #define AR_CORE_ERROR_TAG(tag, ...)
    #define AR_CORE_CRITICAL_TAG(tag, ...)

    #define AR_CORE_TRACE(...)
    #define AR_CORE_INFO(...)
    #define AR_CORE_DEBUG(...)
    #define AR_CORE_WARN(...)
    #define AR_CORE_ERROR(...)
    #define AR_CORE_CRITICAL(...)

    #define AR_TRACE_TAG(tag, ...)
    #define AR_INFO_TAG(tag, ...)
    #define AR_DEBUG_TAG(tag, ...)
    #define AR_WARN_TAG(tag, ...)
    #define AR_ERROR_TAG(tag, ...)
    #define AR_CRITICAL_TAG(tag, ...)

    #define AR_TRACE(...)
    #define AR_INFO(...)
    #define AR_DEBUG(...)
    #define AR_WARN(...)
    #define AR_ERROR(...)
    #define AR_CRITICAL(...)

#endif