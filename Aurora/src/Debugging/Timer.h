#pragma once

// TODO: add a way to globally store all timers created and then displaying their outputs through a dedicated ImGui window
// This is to be done throught a macro like AR_PERFRAME_TIME(name), where the timers results will be added to a global per frame cumilative
// list. And that is what to be displayed in the UI and sort them in descending order and add red indicators for long times

#include <map> // Could use unordered map however the sorting has to be done manually

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

		double ElapsedMillis()
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
			float time = (float)m_Timer.ElapsedMillis();
			AR_CORE_TRACE("[TIMER] {0} - {1}ms", m_Name, time);
		}

	private:
		const char* m_Name;
		Timer m_Timer;
	};

	class PerformanceTimer
	{
	public:
		void SetPerFrameTiming(const char* name, float time)
		{
			m_Name = name;
			s_Total.time = time;
		}

		~PerformanceTimer()
		{
			float time = (float)m_Timer.ElapsedMillis();
			s_Total.time += time;
		}

		static void Reset() { s_Total.time = 0; }

		static float GetTotalTime() { return s_Total.time; }

	private:
		struct TotalTime
		{
			float time = 0;
		};
		static TotalTime s_Total;

		const char* m_Name;
		Timer m_Timer;
	};

}

// For example i could use the below macro in a function that i want to time then in the end of the frame i could do:
// AR_INFO("TOTAL TIME: {0}", PerformanceTimer::GetTotalTime()); which will return the total time that function took throughtout the
// whole frame, even if it was called n times, it return the time it took throughtout all these n times

#define AR_PERF_TIMER(name) ::Aurora::PerformanceTimer timer; timer.SetPerFrameTiming(name, PerformanceTimer::GetTotalTime())
#define AR_ENDF_TIMER()     ::Aurora::PerformanceTimer::Reset()