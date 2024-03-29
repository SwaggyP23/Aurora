#include "Aurorapch.h"
#include "Log.h"

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

namespace Aurora {

	namespace Logger {

		std::shared_ptr<spdlog::logger> Log::s_CoreLogger;
		std::shared_ptr<spdlog::logger> Log::s_ClientLogger;

		void Log::Init() 
		{
			Timer timer;
			std::vector<spdlog::sink_ptr> logSinks;
			logSinks.emplace_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
			logSinks.emplace_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>("../Aurora/LogDump/Aurora.log", true));

			logSinks[0]->set_pattern("%^[%T] %n(%l): %v%$");
			logSinks[1]->set_pattern("[%T] [%l] %n: %v");

			s_CoreLogger = std::make_shared<spdlog::logger>("Aurora", begin(logSinks), end(logSinks));
			spdlog::register_logger(s_CoreLogger);
			s_CoreLogger->set_level(spdlog::level::trace);
			s_CoreLogger->flush_on(spdlog::level::trace);

			s_ClientLogger = std::make_shared<spdlog::logger>("Client", begin(logSinks), end(logSinks));
			spdlog::register_logger(s_ClientLogger);
			s_ClientLogger->set_level(spdlog::level::trace);
			s_ClientLogger->flush_on(spdlog::level::trace);

			AR_CORE_DEBUG_TAG("Log", "Initialized logger! Initialization took : {}ms", timer.ElapsedMillis());
		}

		void Log::ShutDown()
		{
			s_CoreLogger.reset();
			s_ClientLogger.reset();
			spdlog::drop_all();
		}
	}

}