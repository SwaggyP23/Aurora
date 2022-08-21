#pragma once

#include "Graphics/Camera.h"

namespace Aurora {

	// TODO: Add a perspective camera
	class SceneCamera : public Camera
	{
	public:
		enum class ProjectionType { Perspective = 0, Orthographic = 1 };

	public:
		SceneCamera();
		virtual ~SceneCamera() = default;

		void SetViewportSize(uint32_t width, uint32_t height);

		void SetPerspective(float verticalFov, float nearClip, float farClip);
		void SetOrthographic(float size, float nearClip, float farClip);

		float GetPerspectiveVerticalFOV() const { return m_PerpectiveFOV; }
		void SetPerspectiveVerticalFOV(float verticalFov) { m_PerpectiveFOV = verticalFov; RecalculateProjection(); }
		float GetPerspectiveNearClip() const { return m_PerpectiveNear; }
		void SetPerspectiveNearClip(float nearClip) { m_PerpectiveNear = nearClip; RecalculateProjection(); }
		float GetPerspectiveFarClip() const { return m_PerspectiveFar; }
		void SetPerspectiveFarClip(float farClip) { m_PerspectiveFar = farClip;  RecalculateProjection(); }

		float GetOrthographicSize() const { return m_OrthoSize; }
		void SetOrthographicSize(float size) { m_OrthoSize = size; RecalculateProjection(); }
		float GetOrthographicNearClip() const { return m_OrthoNear; }
		void SetOrthographicNearClip(float nearClip) { m_OrthoNear = nearClip; RecalculateProjection(); }
		float GetOrthographicFarClip() const { return m_OrthoFar; }
		void SetOrthographicFarClip(float farClip) { m_OrthoFar = farClip;  RecalculateProjection(); }

		ProjectionType GetProjectionType() const { return m_ProjectionType; }
		void SetProjectionType(ProjectionType type) { m_ProjectionType = type;  RecalculateProjection(); }

	private:
		void RecalculateProjection();

	private:
		ProjectionType m_ProjectionType = ProjectionType::Perspective;

		float m_PerpectiveFOV = glm::radians(45.0f);
		float m_PerpectiveNear = 0.01f, m_PerspectiveFar = 1000.0f;

		float m_OrthoSize = 10.0f;
		float m_OrthoNear = -1.0f, m_OrthoFar = 1.0f;

		float m_AspectRatio = 0.0f;
	};

}