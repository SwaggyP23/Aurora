#pragma once

#include "Graphics/Camera.h"
#include "Core/TimeStep.h"
#include "Core/Events/Events.h"
#include "Core/Events/MouseEvents.h"

#include <glm/glm.hpp>

// Taken Completely from Hazel-dev with some extra tweaks
// https://hazelengine.com/

namespace Aurora {

	enum class CameraType
	{
		BlenderCam = 0, FirstPerson
	};

	class EditorCamera : public Camera
	{
	public:
		EditorCamera() = default;
		EditorCamera(float degFov, float width, float height, float nearClip, float farClip);
		virtual ~EditorCamera() = default;

		void Init();

		void OnUpdate(TimeStep ts);
		void OnEvent(Event& e);

		void Focus(const glm::vec3& focusPoint);

		void SetViewportSize(uint32_t width, uint32_t height);

		[[nodiscard]] float GetFOV() const { return m_FOV; }
		[[nodiscard]] float GetAspectRatio() const { return m_AspectRatio; }
		[[nodiscard]] float GetNearClip() const { return m_NearClip; }
		[[nodiscard]] float GetFarClip() const { return m_FarClip; }

		[[nodiscard]] float GetCameraSpeed() const;

		[[nodiscard]] glm::vec3 GetUpDirection() const;
		[[nodiscard]] glm::vec3 GetRightDirection() const;
		[[nodiscard]] glm::vec3 GetForwardDirection() const;
		[[nodiscard]] glm::quat GetOrientation() const;

		[[nodiscard]] const glm::vec3& GetPosition() const { return m_Position; }

		[[nodiscard]] inline float GetDistance() const { return m_Distance; }
		inline void SetDistance(float distance) { m_Distance = distance; }

		[[nodiscard]] const glm::mat4& GetViewMatrix() const { return m_ViewMatrix; }
		[[nodiscard]] glm::mat4 GetViewProjection() const { return GetProjection() * m_ViewMatrix; }

		[[nodiscard]] float GetPitch() const { return m_Pitch; }
		[[nodiscard]] float GetYaw() const { return m_Yaw; }

		inline void SetActive(bool active) { m_IsActive = active; }
		[[nodiscard]] inline bool IsActive() const { return m_IsActive; }

	private:
		void UpdateView();

		bool OnMouseScroll(MouseScrolledEvent& e);

		void MousePan(const glm::vec2& delta);
		void MouseRotate(const glm::vec2& delta);
		void MouseZoom(float delta);

		std::pair<float, float> PanSpeed() const;
		float RotationSpeed() const;
		float ZoomSpeed() const;

		glm::vec3 CalculatePosition() const;

	private:
		float m_FOV = 45.0f;
		float m_AspectRatio = 1.778f;
		float m_NearClip = 0.1f;
		float m_FarClip = 1000.0f;

		glm::mat4 m_ViewMatrix;

		glm::vec3 m_FocalPoint = glm::vec3{ 0.0f };
		glm::vec3 m_FocalPointDelta = glm::vec3{ 0.0f };

		glm::vec3 m_Position = glm::vec3{ 0.0f };
		glm::vec3 m_PositionDelta = glm::vec4{ 0.0f };

		glm::vec3 m_Direction;
		glm::vec3 m_RightDirection;

		glm::vec2 m_InitialMousePosition = glm::vec2{ 0.0f };

		bool m_IsActive = false;

		float m_Distance = 25.0f;

		float m_NormalSpeed = 0.002f;

		float m_Pitch = 0.25f;
		float m_PitchDelta = 0.0f;
		float m_Yaw = 0.0f;
		float m_YawDelta = 0.0f;

		float m_MinFocusDistance = 100.0f;

		CameraType m_CameraType = CameraType::BlenderCam;

		uint32_t m_ViewportWidth = 1280, m_ViewportHeight = 720;

		constexpr static float MIN_SPEED = 0.0005f;
		constexpr static float MAX_SPEED = 2.0f;
	};

}