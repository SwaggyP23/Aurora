#pragma once

#include "Logging/Log.h"

#include <unordered_map>

namespace Aurora {

	class Timer
	{
	private:
		using HighResClock = std::chrono::high_resolution_clock;
		using NanoSeconds = std::chrono::nanoseconds;

	public:
		Timer()
		{
			Reset();
		}

		void Timer::Reset()
		{
			m_Start = HighResClock::now();
		}

		float Timer::Elapsed() // Returns time in seconds
		{
			return std::chrono::duration_cast<NanoSeconds>(HighResClock::now() - m_Start).count() * 0.001f * 0.001f * 0.001f;
		}

		float Timer::ElapsedMillis() // in milliseconds
		{
			return Elapsed() * 1000.0f;
		}

	private:
		std::chrono::time_point<HighResClock> m_Start;
	};

	class ScopedTimer
	{
	public:
		ScopedTimer(const char* name)
			: m_Name(name)
		{
		}

		~ScopedTimer()
		{
			AR_CORE_TRACE_TAG("TIMER", "{0} - {1} milliSecs", m_Name, (float)m_Timer.ElapsedMillis());
		}

	private:
		const char* m_Name;
		Timer m_Timer;
	};

	class PerformanceProfiler
	{
	public:
		void SetPerFrameTiming(const char* name, float time)
		{
			if (m_PerFrameData.find(name) == m_PerFrameData.end())
				m_PerFrameData[name] = 0.0f;

			m_PerFrameData[name] += time;
		}

		void Clear() { m_PerFrameData.clear(); }
		const std::unordered_map<const char*, float>& GetPerFrameData() const { return m_PerFrameData; }

	private:
		std::unordered_map<const char*, float> m_PerFrameData;

	};

	class PerFrameTimer
	{
	public:
		PerFrameTimer(const char* name, PerformanceProfiler* profiler)
			: m_Name(name), m_Profiler(profiler) {}

		~PerFrameTimer()
		{
			float time = (float)m_Timer.ElapsedMillis();
			m_Profiler->SetPerFrameTiming(m_Name, time);
		}

	private:
		const char* m_Name;
		PerformanceProfiler* m_Profiler;
		Timer m_Timer;

	};

}

#define AR_SCOPE_PERF(name)      Aurora::PerFrameTimer AR_CONCAT_MACRO(timer, __LINE__)(name, Aurora::Application::GetApp().GetPerformanceProfiler())
#define AR_SCOPED_TIMER(name)    Aurora::ScopedTimer AR_CONCAT_MACRO(timer, __LINE__)(name)