#pragma once

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
			s_RandomEngine.seed(std::random_device()());
		}

		// Returns a 32-bit random number
		static uint32_t UInt()
		{
			return s_Distribution(s_RandomEngine);
		}

		// Returns a random unsiged integer in the specified range
		static uint32_t UInt(uint32_t min, uint32_t max)
		{
			// This should be a modulus and not multiplication since the random number generated is not in the 0 -> 1 range
			return min + (s_Distribution(s_RandomEngine) % (max - min + 1));
		}
		
		// Returns a random float between 0 and 1
		static float Float()
		{
			return ((float)s_Distribution(s_RandomEngine) / (float)std::numeric_limits<uint32_t>::max());
		}

		// Returns a random float in the specified range
		static float Float(float min, float max)
		{
			// Unlike in the UInt() function, here we have to multiply since the generated float is in the 0 -> 1 range
			return min + Float() * (max - min) + 1;
		}

		// Returns a vec2 with 2 random floats
		static glm::vec2 Vec2()
		{
			return glm::vec2(Float(), Float());
		}

		// Returns a vec2 with 2 random floats in the specified range
		static glm::vec2 Vec2(float min, float max)
		{
			return glm::vec2(Float(min, max), Float(min, max));
		}

		// Returns a vec3 with 3 random floats
		static glm::vec3 Vec3()
		{
			return glm::vec3(Float(), Float(), Float());
		}

		// Returns a vec3 with 3 random floats in the specified range
		static glm::vec3 Vec3(float min, float max)
		{
			return glm::vec3(Float(min, max), Float(min, max), Float(min, max));
		}

		// Returns a vec4 with 4 random floats
		static glm::vec4 Vec4()
		{
			return glm::vec4(Float(), Float(), Float(), Float());
		}

		// Returns a vec4 with 4 random floats in the specified range
		static glm::vec4 Vec4(float min, float max)
		{
			return glm::vec4(Float(min, max), Float(min, max), Float(min, max), Float(min, max));
		}

	private:
		static std::mt19937 s_RandomEngine;
		static std::uniform_int_distribution<std::mt19937::result_type> s_Distribution;

	};

}