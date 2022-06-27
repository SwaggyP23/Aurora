#pragma once

#include "OGLpch.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UPWARD,
    DOWNWARD
};

// Default camera values
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 15.0f;
const float SENSITIVITY = 0.2f;
const float ZOOM = 90.0f;


// An abstract camera class that processes input and calculates the corresponding Euler Angles, Vectors and Matrices for use in OpenGL
class Camera
{
public:
    // constructor with vectors
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float m_Yaw = YAW, float pitch = PITCH);
    // constructor with scalar values
    Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float m_Yaw, float pitch);

    glm::mat4 GetViewMatrix();

    void ProcessKeyboard(Camera_Movement direction, float deltaTime);
    void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch = true);
    void ProcessMouseScroll(float yoffset);

    inline float& getZoom() { return m_Zoom; }
    inline bool getFirstMouse() const { return m_FirstMouse; }
    inline void setFirstMouse(bool state) { m_FirstMouse = state; }
    inline float getLastx() const { return m_Lastx; }
    inline float getLasty() const { return m_Lasty; }
    inline void setLastx(float var) { m_Lastx = var; }
    inline void setLasty(float var) { m_Lasty = var; }

private:
    // calculates the front vector from the Camera's (updated) Euler Angles
    void updateCameraVectors();

private:
    glm::vec3 m_Position;
    glm::vec3 m_Front;
    glm::vec3 m_Up;
    glm::vec3 m_Right;
    glm::vec3 m_WorldUp;
    // euler Angles
    float m_Yaw;
    float m_Pitch;
    // camera options
    float m_MovementSpeed;
    float m_MouseSensitivity;
    float m_Zoom;
    float m_Lastx = 1280 / 2;
    float m_Lasty = 720 / 2;
    bool m_FirstMouse = true;

};