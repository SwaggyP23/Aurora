#pragma once

// AXIS-ALIGNED BOUNDING BOX (AABB)

#include <glm/glm.hpp>

namespace Aurora {

	struct AABB
	{
		AABB() 
			: Min(0.0f), Max(0.0f) {}
		AABB(const glm::vec3& min, const glm::vec3& max)
			: Min(min), Max(max) {}


		glm::vec3 Min = glm::vec3(0.0f);
		glm::vec3 Max = glm::vec3(0.0f);
	};

}