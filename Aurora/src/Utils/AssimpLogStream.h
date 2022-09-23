#pragma once

#include "Logging/Log.h"

#include <assimp/LogStream.hpp>
#include <assimp/DefaultLogger.hpp>

namespace Aurora {

	class AssimpLogStream : public Assimp::LogStream
	{
	public:
		static void Init()
		{
			if (Assimp::DefaultLogger::isNullLogger())
			{
				Assimp::DefaultLogger::create("", Assimp::Logger::VERBOSE);
				uint32_t severity = Assimp::Logger::Debugging | Assimp::Logger::Info | Assimp::Logger::Warn | Assimp::Logger::Err;
				Assimp::DefaultLogger::get()->attachStream(new AssimpLogStream(), severity);
			}
		}

		static void Shutdown()
		{
			Assimp::DefaultLogger::kill();
		}

		virtual void write(const char* message) override
		{
			std::string msg(message);
			if (!msg.empty() && msg[msg.length() - 1] == '\n')
			{
				msg.erase(msg.length() - 1);
			}
			if (strncmp(message, "Debug", 5) == 0)
			{
				AR_CORE_DEBUG_TAG("Assimp", msg);
			}
			else if (strncmp(message, "Info", 5) == 0)
			{
				AR_CORE_INFO_TAG("Assimp", msg);
			}
			else if (strncmp(message, "Warn", 5) == 0)
			{
				AR_CORE_WARN_TAG("Assimp", msg);
			}
			else
			{
				AR_CORE_ERROR_TAG("Assimp", msg);
			}
		}

	};

}