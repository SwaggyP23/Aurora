#include "Aurorapch.h"
#include "Log.h"

#include "Editor/EditorConsoleSink.h"

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

#define AR_HAS_CONSOLE !AURORA_DIST

namespace Aurora {

	namespace Logger {

		std::shared_ptr<spdlog::logger> Log::s_CoreLogger;
		std::shared_ptr<spdlog::logger> Log::s_ClientLogger;
		std::shared_ptr<spdlog::logger> Log::s_EditorConsoleLogger;

		void Log::Init() 
		{
			std::vector<spdlog::sink_ptr> auroraSinks =
			{
				std::make_shared<spdlog::sinks::basic_file_sink_mt>("../Aurora/LogDump/AURORA.log", true)
#if AR_HAS_CONSOLE
				,
				std::make_shared<spdlog::sinks::stdout_color_sink_mt>(),
#endif
			};

			std::vector<spdlog::sink_ptr> appSinks =
			{
				std::make_shared<spdlog::sinks::basic_file_sink_mt>("../Aurora/LogDump/APP.log", true)
#if AR_HAS_CONSOLE
				,
				std::make_shared<spdlog::sinks::stdout_color_sink_mt>()
#endif
			};

			std::vector<spdlog::sink_ptr> editorSinks =
			{
				std::make_shared<spdlog::sinks::basic_file_sink_mt>("../Aurora/LogDump/APP.log", true),
				// Create it with bufferCapacitiy == 1 so it flushes everytime there is a message
				std::make_shared<EditorConsoleSink>(1),
			};

			// File sinks
			auroraSinks[0]->set_pattern("[%T] [%l] %n: %v");
			appSinks[0]->set_pattern("[%T] [%l] %n: %v");

#if AR_HAS_CONSOLE
			auroraSinks[1]->set_pattern("%^[%T] %n(%l): %v%$");
			appSinks[1]->set_pattern("%^[%T] %n(%l): %v%$");
#endif
			for (spdlog::sink_ptr sink : editorSinks)
				sink->set_pattern("%^%v%$");

			s_CoreLogger = std::make_shared<spdlog::logger>("Aurora", begin(auroraSinks), end(auroraSinks));
			spdlog::register_logger(s_CoreLogger);
			s_CoreLogger->set_level(spdlog::level::trace);

			s_ClientLogger = std::make_shared<spdlog::logger>("Client", begin(appSinks), end(appSinks));
			spdlog::register_logger(s_ClientLogger);
			s_ClientLogger->set_level(spdlog::level::trace);

			s_EditorConsoleLogger = std::make_shared<spdlog::logger>("Console", begin(editorSinks), end(editorSinks));
			spdlog::register_logger(s_EditorConsoleLogger);
			s_EditorConsoleLogger->set_level(spdlog::level::trace);
		}

		void Log::ShutDown()
		{
			s_CoreLogger.reset();
			s_ClientLogger.reset();
			s_EditorConsoleLogger.reset();
			spdlog::drop_all();
		}
	}

}