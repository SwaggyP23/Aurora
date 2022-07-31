#pragma once

#include <unordered_map>

#include "Logging/Log.h"

namespace Aurora {

	class Timer
	{
	public:
		Timer()
		{
			Reset();
		}

		void Reset()
		{
			m_Start = std::chrono::high_resolution_clock::now();
		}

		double Elapsed() // returns time in seconds
		{
			return std::chrono::duration_cast<std::chrono::duration<float, std::milli>>(std::chrono::high_resolution_clock::now() - m_Start).count() / 1000.0f;
			//return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - m_Start).count() * 0.001f * 0.001f * 0.001f;
		}

		float ElapsedMillis() // returns time in microseconds
		{
			return (float)Elapsed() * 1000.0f;
		}

	private:
		std::chrono::time_point<std::chrono::high_resolution_clock> m_Start;
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
			AR_CORE_TRACE("[TIMER] {0} - {1} milliSecs", m_Name, (float)m_Timer.ElapsedMillis());
		}

	private:
		const char* m_Name;
		Timer m_Timer;
	};

	class PerformanceTimer
	{
	public:
		PerformanceTimer(std::string name)
			: m_Name(name), m_Timer() {}

		void SetPerFrameTime(float time)
		{
			m_Time = time;
		}

		~PerformanceTimer()
		{
			float time = (float)m_Timer.ElapsedMillis();
			m_Time += time;
			s_TimeMap[m_Name] = m_Time;
		}

		float GetPreviousTime() { return m_Time; }

		static std::unordered_map<std::string, float>& GetTimeMap() { return s_TimeMap; }

	private:
		std::string m_Name;
		Timer m_Timer;
		float m_Time = 0;
		float m_Threshold = 3.0f;

		static std::unordered_map<std::string, float> s_TimeMap;
	};

}

#define AR_PERF_TIMER(name)     ::Aurora::PerformanceTimer AR_CONCAT_MACRO(timer, __LINE__)(name); AR_CONCAT_MACRO(timer, __LINE__).SetPerFrameTime(AR_CONCAT_MACRO(timer, __LINE__).GetPreviousTime())
#define AR_ENDF_TIMER()         for(auto&[name, time] : PerformanceTimer::GetTimeMap()) { time = 0; }