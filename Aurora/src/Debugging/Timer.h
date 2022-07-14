#pragma once

// TODO: add a way to globally store all timers created and then displaying their outputs through a dedicated ImGui window
// This is to be done throught a macro like AR_PERFRAME_TIME(name), where the timers results will be added to a global per frame cumilative
// list. And that is what to be displayed in the UI and sort them in descending order and add red indicators for long times

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
		ScopedTimer(const std::string& name)
			: m_Name(name)
		{
		}

		~ScopedTimer()
		{
			float time = (float)m_Timer.ElapsedMillis();
			AR_CORE_TRACE("[TIMER] {0} - {1}ms", m_Name, time);
		}

	private:
		std::string m_Name;
		Timer m_Timer;
	};

}