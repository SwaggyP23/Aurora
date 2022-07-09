#include "Aurorapch.h"
#include "OrthoGraphicCamera.h"

#include "Input/Input.h"

#include <glfw/glfw3.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

OrthoGraphicCamera::OrthoGraphicCamera(float aspectRatio, float nearClip, float farClip)
	: m_AspectRatio(aspectRatio), m_NearClip(nearClip), m_FarClip(farClip)
{
	m_Projection = glm::ortho(-m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -1.0f, 1.0f, m_NearClip, m_FarClip);
	UpdateView();
}

void OrthoGraphicCamera::UpdateProjection()
{
	PROFILE_FUNCTION();

	m_AspectRatio = m_ViewportWidth / m_ViewportHeight;
	//m_Projection = glm::perspective(glm::radians(16.0f / 9.0f), m_AspectRatio, m_NearClip, m_FarClip);
	m_Projection = glm::ortho(-m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -1.0f, 1.0f, m_NearClip, m_FarClip);
}

void OrthoGraphicCamera::UpdateView()
{
	PROFILE_FUNCTION();

	// m_Yaw = m_Pitch = 0.0f; // Lock the camera's rotation
	m_Position = CalculatePosition();

	glm::quat orientation = GetOrientation();
	m_ViewMatrix = glm::translate(glm::mat4(1.0f), m_Position) * glm::toMat4(orientation);
	m_ViewMatrix = glm::inverse(m_ViewMatrix);
}

std::pair<float, float> OrthoGraphicCamera::PanSpeed() const
{
	float x = std::min(m_ViewportWidth / 1000.0f, 2.4f); // max = 2.4f
	float xFactor = 0.0366f * (x * x) - 0.1778f * x + 0.3021f;

	float y = std::min(m_ViewportHeight / 1000.0f, 2.4f); // max = 2.4f
	float yFactor = 0.0366f * (y * y) - 0.1778f * y + 0.3021f;

	return { xFactor, yFactor };
}

float OrthoGraphicCamera::RotationSpeed() const
{
	return 0.8f;
}

float OrthoGraphicCamera::ZoomSpeed() const
{
	float distance = m_Distance * 0.2f;
	distance = std::max(distance, 0.0f);
	float speed = distance * distance;
	speed = std::min(speed, 100.0f); // max speed = 100
	return speed;
}

void OrthoGraphicCamera::OnUpdate(TimeStep ts)
{
	PROFILE_FUNCTION();

	if (Input::isKeyPressed(GLFW_KEY_LEFT_CONTROL))
	{
		const glm::vec2& mouse{ Input::getMouseX(), Input::getMouseY() };
		glm::vec2 delta = (mouse - m_InitialMousePosition) * 0.003f;
		m_InitialMousePosition = mouse;

		if (Input::isMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT))
			MousePan(delta);
		else if (Input::isMouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT))
			MouseRotate(delta);
		else if (Input::isMouseButtonPressed(GLFW_MOUSE_BUTTON_MIDDLE))
			MouseZoom(delta.y);
	}

	UpdateView();
}

void OrthoGraphicCamera::OnEvent(Event& e)
{
	PROFILE_FUNCTION();

	EventDispatcher dispatcher(e);
	dispatcher.dispatch<MouseScrolledEvent>(SET_EVENT_FN(OrthoGraphicCamera::OnMouseScroll));
}

bool OrthoGraphicCamera::OnMouseScroll(MouseScrolledEvent& e)
{
	PROFILE_FUNCTION();

	if (Input::isKeyPressed(GLFW_KEY_LEFT_CONTROL)) {
		//float delta = e.getYOffset() * 0.1f;
		//MouseZoom(delta);
		//UpdateView();

		m_ZoomLevel -= e.getYOffset() * 0.25f;
		m_ZoomLevel = std::max(m_ZoomLevel, 0.25f);
		m_Projection = glm::ortho(-m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel, m_NearClip, m_FarClip);
	}
	return false;
}

void OrthoGraphicCamera::MousePan(const glm::vec2& delta)
{
	auto [xSpeed, ySpeed] = PanSpeed();
	m_FocalPoint += -GetRightDirection() * delta.x * xSpeed * m_Distance;
	m_FocalPoint += GetUpDirection() * delta.y * ySpeed * m_Distance;
}

void OrthoGraphicCamera::MouseRotate(const glm::vec2& delta)
{
	float yawSign = GetUpDirection().y < 0 ? -1.0f : 1.0f;
	m_Yaw += yawSign * delta.x * RotationSpeed();
	m_Pitch += delta.y * RotationSpeed();
}

void OrthoGraphicCamera::MouseZoom(float delta)
{
	m_Distance -= delta * ZoomSpeed();
	if (m_Distance < 1.0f)
	{
		m_FocalPoint += GetForwardDirection();
		m_Distance = 1.0f;
	}
}

glm::vec3 OrthoGraphicCamera::GetUpDirection() const
{
	return glm::rotate(GetOrientation(), glm::vec3(0.0f, 1.0f, 0.0f));
}

glm::vec3 OrthoGraphicCamera::GetRightDirection() const
{
	return glm::rotate(GetOrientation(), glm::vec3(1.0f, 0.0f, 0.0f));
}

glm::vec3 OrthoGraphicCamera::GetForwardDirection() const
{
	return glm::rotate(GetOrientation(), glm::vec3(0.0f, 0.0f, -1.0f));
}

glm::vec3 OrthoGraphicCamera::CalculatePosition() const
{
	return m_FocalPoint - GetForwardDirection() * m_Distance;
}

glm::quat OrthoGraphicCamera::GetOrientation() const
{
	return glm::quat(glm::vec3(-m_Pitch, -m_Yaw, 0.0f));
}