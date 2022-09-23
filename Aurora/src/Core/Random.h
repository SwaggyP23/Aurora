#pragma once

#include "Base.h"

#include <random>
#include <glm/glm.hpp>

/*
 * This randomEngine returns a random float between 0 and 1, you can multiply the result by 5 for example to get a random number
 * between 0 and 5...
 * We could also use the random engine mt19937_64 which gives 64-bit random numbers however that will mostly be used for UUIDS/GUIDS
 */

namespace Aurora {

	class Random
	{
	public:
		static void Init()
		{
			std::random_device randomDevice;
			s_RandomEngine32.seed(randomDevice());
			s_RandomEngine64.seed(randomDevice());
		}

		// Returns a 32-bit random number
		[[nodiscard]] static uint32_t UInt32()
		{
			return s_Distribution32(s_RandomEngine32);
		}

		// Returns a random unsiged integer in the specified range
		[[nodiscard]] static uint32_t UInt32(uint32_t min, uint32_t max)
		{
			// This should be a modulus and not multiplication since the random number generated is not in the 0 -> 1 range
			return min + (s_Distribution32(s_RandomEngine32) % (max - min + 1));
		}

		// Returns a 64-bit random number
		[[nodiscard]] static uint64_t UInt64()
		{
			return s_Distribution64(s_RandomEngine64);
		}

		// Returns a random unsiged long long in the specified range
		[[nodiscard]] static uint64_t UInt64(uint64_t min, uint64_t max)
		{
			// This should be a modulus and not multiplication since the random number generated is not in the 0 -> 1 range
			return min + (s_Distribution64(s_RandomEngine64) % (max - min + 1));
		}
		
		// Returns a random float between 0 and 1
		[[nodiscard]] static float Float()
		{
			return ((float)s_Distribution32(s_RandomEngine32) / (float)std::numeric_limits<uint32_t>::max());
		}

		// Returns a random double between 0 and 1
		[[nodiscard]] static double Double()
		{
			return ((double)s_Distribution64(s_RandomEngine64) / (double)std::numeric_limits<uint64_t>::max());
		}

		// Returns a random float in the specified range
		[[nodiscard]] static float Float(float min, float max)
		{
			// Unlike in the UInt() function, here we have to multiply since the generated float is in the 0 -> 1 range
			return min + Float() * (max - min) + 1;
		}

		// Returns a random float in the specified range
		[[nodiscard]] static double Double(double min, double max)
		{
			// Unlike in the UInt() function, here we have to multiply since the generated double is in the 0 -> 1 range
			return min + Double() * (max - min) + 1;
		}

		// Returns a vec2 with 2 random floats
		[[nodiscard]] static glm::vec2 Vec2()
		{
			return glm::vec2(Float(), Float());
		}

		// Returns a vec2 with 2 random floats in the specified range
		[[nodiscard]] static glm::vec2 Vec2(float min, float max)
		{
			return glm::vec2(Float(min, max), Float(min, max));
		}

		// Returns a vec3 with 3 random floats
		[[nodiscard]] static glm::vec3 Vec3()
		{
			return glm::vec3(Float(), Float(), Float());
		}

		// Returns a vec3 with 3 random floats in the specified range
		[[nodiscard]] static glm::vec3 Vec3(float min, float max)
		{
			return glm::vec3(Float(min, max), Float(min, max), Float(min, max));
		}

		// Returns a vec4 with 4 random floats
		[[nodiscard]] static glm::vec4 Vec4()
		{
			return glm::vec4(Float(), Float(), Float(), Float());
		}

		// Returns a vec4 with 4 random floats in the specified range
		[[nodiscard]] static glm::vec4 Vec4(float min, float max)
		{
			return glm::vec4(Float(min, max), Float(min, max), Float(min, max), Float(min, max));
		}

	private:
		static std::mt19937 s_RandomEngine32;
		static std::mt19937_64 s_RandomEngine64;
		static std::uniform_int_distribution<std::mt19937::result_type> s_Distribution32;
		static std::uniform_int_distribution<std::mt19937_64::result_type> s_Distribution64;

	};

}