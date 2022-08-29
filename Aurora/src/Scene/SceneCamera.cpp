#include "Aurorapch.h"
#include "SceneCamera.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Aurora {

	void SceneCamera::SetViewportSize(uint32_t width, uint32_t height)
	{
		if (m_ProjectionType == ProjectionType::Perspective)
		{
			SetPerspectiveProjectionMatrix(glm::radians(m_DegPerspectiveFOV), (float)width, (float)height, m_PerspectiveNear, m_PerspectiveFar);
		}
		else if (m_ProjectionType == ProjectionType::Orthographic)
		{
			float aspectRatio = (float)width / (float)height;
			float orthoWidth = m_OrthoSize * aspectRatio;
			float orthoHeight = m_OrthoSize;
			SetOrthographicProjectionMatrix(orthoWidth, orthoHeight, m_OrthoNear, m_OrthoFar);
		}
	}

	void SceneCamera::SetPerspective(float degVerticalFov, float nearClip, float farClip)
	{
		m_ProjectionType = ProjectionType::Perspective;
		m_DegPerspectiveFOV = degVerticalFov;
		m_PerspectiveNear = nearClip;
		m_PerspectiveFar = farClip;
	}

	void SceneCamera::SetOrthographic(float size, float nearClip, float farClip)
	{
		m_ProjectionType = ProjectionType::Orthographic;
		m_OrthoSize = size;
		m_OrthoNear = nearClip;
		m_OrthoFar = farClip;
	}

}