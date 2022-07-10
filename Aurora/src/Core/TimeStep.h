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

		inline float getSeconds() const { return m_Time; }
		inline float getMilliSeconds() const { return m_Time * 1000.0f; }

	private:
		float m_Time;
	};

}