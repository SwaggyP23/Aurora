#pragma once

namespace Aurora {

	class TimeStep
	{
	public:
		TimeStep(float time = 0.0f)
			: m_Time(time)
		{
		}

		operator float() const { return m_Time; }

		[[nodiscard]] inline float GetSeconds() const { return m_Time; }
		[[nodiscard]] inline float GetMilliSeconds() const { return m_Time * 1000.0f; }

	private:
		float m_Time = 0.0f;
	};

}