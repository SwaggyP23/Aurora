#pragma once

#include <map> // Could use unordered_map, however this provides automatic sorting which is conveniant for the use case

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
			return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - m_Start).count() * 0.000000001;
		}

		double ElapsedMicros() // returns time in microseconds
		{
			return Elapsed() * 1000.0f;
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
			AR_CORE_TRACE("[TIMER] {0} - {1}milliSecs", m_Name, (float)m_Timer.ElapsedMicros());
		}

	private:
		const char* m_Name;
		Timer m_Timer;
	};

	class PerformanceTimer
	{
	public:
		void SetPerFrameTime(const char* name, float time)
		{
			m_Name = name;
			m_Time = time;
		}

		~PerformanceTimer()
		{
			float time = (float)m_Timer.ElapsedMicros();
			m_Time += time;
			s_TimeMap[m_Name] = m_Time;
		}

		float GetPreviousTime() { return m_Time; }

		static std::map<std::string, float>& GetTimeMap() { return s_TimeMap; }

	private:
		std::string m_Name;
		Timer m_Timer;
		float m_Time = 0;

		static std::map<std::string, float> s_TimeMap;
	};

}

#define AR_PERF_TIMER(name) ::Aurora::PerformanceTimer timer##__LINE__; timer##__LINE__.SetPerFrameTime(name, timer##__LINE__.GetPreviousTime())
#define AR_ENDF_TIMER       for(auto&[name, time] : PerformanceTimer::GetTimeMap()) { time = 0; }