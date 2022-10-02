#include "Aurorapch.h"
#include "Log.h"

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

namespace Aurora {

	namespace Logger {

		std::shared_ptr<spdlog::logger> Log::s_CoreLogger;
		std::shared_ptr<spdlog::logger> Log::s_ClientLogger;

		// Takes about 1.3ms on average to initialize
		void Log::Init() 
		{
			std::vector<spdlog::sink_ptr> auroraSinks;
			auroraSinks.emplace_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
			auroraSinks.emplace_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>("../Aurora/LogDump/AURORA.log", true));

			std::vector<spdlog::sink_ptr> appSinks;
			appSinks.emplace_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
			appSinks.emplace_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>("../Aurora/LogDump/APP.log", true));

			auroraSinks[0]->set_pattern("%^[%T] %n(%l): %v%$");
			auroraSinks[1]->set_pattern("[%T] [%l] %n: %v");

			appSinks[0]->set_pattern("%^[%T] %n(%l): %v%$");
			appSinks[1]->set_pattern("[%T] [%l] %n: %v");

			s_CoreLogger = std::make_shared<spdlog::logger>("Aurora", begin(auroraSinks), end(auroraSinks));
			spdlog::register_logger(s_CoreLogger);
			s_CoreLogger->set_level(spdlog::level::trace);
			s_CoreLogger->flush_on(spdlog::level::trace);

			s_ClientLogger = std::make_shared<spdlog::logger>("Client", begin(appSinks), end(appSinks));
			spdlog::register_logger(s_ClientLogger);
			s_ClientLogger->set_level(spdlog::level::trace);
			s_ClientLogger->flush_on(spdlog::level::trace);
		}

		void Log::ShutDown()
		{
			s_CoreLogger.reset();
			s_ClientLogger.reset();
			spdlog::drop_all();
		}
	}

}