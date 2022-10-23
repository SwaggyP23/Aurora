#pragma once

#include "EditorConsolePanel.h"

#include <spdlog/sinks/base_sink.h>

#include <vector>
#include <mutex>

namespace Aurora {

	class EditorConsoleSink : public spdlog::sinks::base_sink<std::mutex>
	{
	public:
		EditorConsoleSink(uint32_t bufferCapacity)
			: m_MessageBufferCapacity(bufferCapacity), m_MessageBuffer(bufferCapacity) {}
		virtual ~EditorConsoleSink() = default;

		EditorConsoleSink(const EditorConsoleSink&) = delete;
		EditorConsoleSink& operator=(const EditorConsoleSink&) = delete;

	protected:
		void sink_it_(const spdlog::details::log_msg& msg) override
		{
			ConsoleMessage message = {};
			message.Time = std::chrono::system_clock::to_time_t(msg.time);
			message.Flags = GetMessageFlags(msg.level);

			// Refer to: https://github.com/gabime/spdlog/wiki/4.-Sinks#implementing-your-own-sink
			spdlog::memory_buf_t formatted;
			spdlog::sinks::base_sink<std::mutex>::formatter_->format(msg, formatted);
			message.LongMessage = fmt::to_string(formatted);
			message.ShortMessage = message.LongMessage;

			size_t newLinePos = message.ShortMessage.find_first_of('\n');
			if (newLinePos != message.ShortMessage.size() - 1)
			{
				message.ShortMessage.replace(newLinePos, message.ShortMessage.size() - 1, "...");
			}
			else if (message.ShortMessage.size() > 100)
			{
				size_t spacePos = message.ShortMessage.find_first_of(' ', 100);
				if (spacePos != std::string::npos)
					message.ShortMessage.replace(spacePos, message.ShortMessage.size() - 1, "...");
			}

			m_MessageBuffer[m_MessageCount++] = message;

			if (m_MessageCount == m_MessageBufferCapacity)
				flush_();
		}

		void flush_() override
		{
			for (const auto& message : m_MessageBuffer)
				EditorConsolePanel::PushMessage(message);

			m_MessageCount = 0;
		}

	private:
		int16_t GetMessageFlags(spdlog::level::level_enum level)
		{
			int16_t flags = 0;

			switch (level)
			{
				// TODO: Maybe add the other types? such is Debug and Critical and have them appear differently in the editor
			    case spdlog::level::trace:
			    case spdlog::level::debug:
			    case spdlog::level::info:
				{
					flags |= (int16_t)ConsoleMessageFlags::Info;
					break;
				}
			    case spdlog::level::warn:
				{
					flags |= (int16_t)ConsoleMessageFlags::Warning;
					break;
				}
			    case spdlog::level::err:
			    case spdlog::level::critical:
				{
					flags |= (int16_t)ConsoleMessageFlags::Error;
					break;
				}
			}

			return flags;
		}

	private:
		uint32_t m_MessageBufferCapacity = 0;
		std::vector<ConsoleMessage> m_MessageBuffer;
		uint32_t m_MessageCount = 0;

	};

}