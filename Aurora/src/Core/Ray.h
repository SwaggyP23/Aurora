#pragma once

#include "AABB.h"

#include <glm/glm.hpp>

namespace Aurora {

	struct Ray
	{
		Ray(const glm::vec3& origin, const glm::vec3 direction)
			: Origin(origin), Direction(direction) {}

		static Ray Zero()
		{
			return { { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } };
		}

		bool IntersectsAABB(const AABB& aabb, float& t) const
		{
			glm::vec3 dirFrac;
			// r.dir is unit direction vector of ray
			dirFrac.x = 1.0f / Direction.x;
			dirFrac.y = 1.0f / Direction.y;
			dirFrac.z = 1.0f / Direction.z;

			// lb is the corner of AABB with minimal coordinates - left bottom, rt is maximal corner
			// r.org is origin of ray
			const glm::vec3& lb = aabb.Min;
			const glm::vec3& rt = aabb.Max;

			float t1 = (lb.x - Origin.x) * dirFrac.x;
			float t2 = (rt.x - Origin.x) * dirFrac.x;
			float t3 = (lb.y - Origin.y) * dirFrac.y;
			float t4 = (rt.y - Origin.y) * dirFrac.y;
			float t5 = (lb.z - Origin.z) * dirFrac.z;
			float t6 = (rt.z - Origin.z) * dirFrac.z;

			float tMin = glm::max(glm::max(glm::min(t1, t2), glm::min(t3, t4)), glm::min(t5, t6));
			float tMax = glm::min(glm::min(glm::max(t1, t2), glm::max(t3, t4)), glm::max(t5, t6));

			// If tMax < 0, ray is intersecing AABB, but the whole AABB is behind us
			if (tMax < 0)
			{
				t = tMax;
				return false;
			}

			// If tMin > tMax, ray doesnt intersect AABB
			if (tMin > tMax)
			{
				t = tMax;
				return false;
			}

			t = tMin;
			
			return true;
		}

		bool IntersectsTriangle(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c, float& t) const
		{
			const glm::vec3 E1 = b - a;
			const glm::vec3 E2 = c - a;
			const glm::vec3 N = glm::cross(E1, E2);
			
			const float det = -glm::dot(Direction, N);
			const float invDet = 1.0f / det;

			const glm::vec3 AO = Origin - a;
			const glm::vec3 DAO = glm::cross(AO, Direction);

			const float u = glm::dot(E2, DAO) * invDet;
			const float v = -glm::dot(E1, DAO) * invDet;

			t = glm::dot(AO, N) * invDet;
			return (det >= 1e-6f && t >= 0.0f && u >= 0.0f && v >= 0.0f && (u + v) <= 1.0f);
		}

		glm::vec3 Origin;
		glm::vec3 Direction;
	};

}