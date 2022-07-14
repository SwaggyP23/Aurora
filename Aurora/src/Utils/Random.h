#pragma once

#include <random>

/*
 * This randomEngine returns a random float between 0 and 1, you can multiply the result by 5 for example to get a random number
 * between 0 and 5...
 * We could also use the random engine mt19937_64 which gives 64-bit random numbers however that will mostly be used for UUIDS/GUIDS
 */

namespace Aurora {

	namespace Utils {

		class Random
		{
		public:
			static void Init()
			{
				s_RandomEngine.seed(std::random_device()());
			}

			static float Float()
			{
				return ((float)s_Distribution(s_RandomEngine) / (float)std::numeric_limits<uint32_t>::max());
			}

		private:
			static std::mt19937 s_RandomEngine;
			static std::uniform_int_distribution<std::mt19937::result_type> s_Distribution;

		};

	}

}