#pragma once

#include "Graphics/Camera.h"

namespace Aurora {

	class SceneCamera : public Camera
	{
	public:
		enum class ProjectionType { Perspective = 0, Orthographic = 1 };

	public:
		virtual ~SceneCamera() = default;

		void SetViewportSize(uint32_t width, uint32_t height);

		void SetPerspective(float degVerticalFov, float nearClip, float farClip);
		void SetOrthographic(float size, float nearClip, float farClip);

		float GetDegPerspectiveVerticalFOV() const { return m_DegPerspectiveFOV; }
		void SetDegPerspectiveVerticalFOV(float degVerticalFov) { m_DegPerspectiveFOV = degVerticalFov; }
		float GetPerspectiveNearClip() const { return m_PerspectiveNear; }
		void SetPerspectiveNearClip(float nearClip) { m_PerspectiveNear = nearClip; }
		float GetPerspectiveFarClip() const { return m_PerspectiveFar; }
		void SetPerspectiveFarClip(float farClip) { m_PerspectiveFar = farClip; }

		float GetOrthographicSize() const { return m_OrthoSize; }
		void SetOrthographicSize(float size) { m_OrthoSize = size; }
		float GetOrthographicNearClip() const { return m_OrthoNear; }
		void SetOrthographicNearClip(float nearClip) { m_OrthoNear = nearClip; }
		float GetOrthographicFarClip() const { return m_OrthoFar; }
		void SetOrthographicFarClip(float farClip) { m_OrthoFar = farClip; }

		ProjectionType GetProjectionType() const { return m_ProjectionType; }
		void SetProjectionType(ProjectionType type) { m_ProjectionType = type; }

	private:
		ProjectionType m_ProjectionType = ProjectionType::Perspective;

		float m_DegPerspectiveFOV = 45.0f;
		float m_PerspectiveNear = 0.1f, m_PerspectiveFar = 1000.0f;

		float m_OrthoSize = 10.0f;
		float m_OrthoNear = -1.0f, m_OrthoFar = 1.0f;

	};

}