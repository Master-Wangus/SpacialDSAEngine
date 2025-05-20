#include "../include/Camera.hpp"
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <cmath>

// Constructor
Camera::Camera(const glm::vec3& position, const glm::vec3& worldUp)
    : m_Position(position),
      m_WorldUp(worldUp),
      m_Yaw(-90.0f),        // Default yaw is -90 degrees (looking forward)
      m_Pitch(0.0f),        // Default pitch is 0 degrees (looking horizontally)
      m_MovementSpeed(2.5f),
      m_MouseSensitivity(0.1f),
      m_Fov(45.0f),
      m_LastX(0.0f),
      m_LastY(0.0f),
      m_FirstMouse(true)
{
    // Initialize camera vectors
    UpdateCameraVectors();
}

void Camera::Update(const Window& window, float deltaTime) 
{
    // Handle keyboard input for camera movement
    float velocity = m_MovementSpeed * deltaTime;
    
    // Forward movement (W key)
    if (window.IsKeyPressed(GLFW_KEY_W)) {
        m_Position += m_Front * velocity;
    }
    
    // Backward movement (S key)
    if (window.IsKeyPressed(GLFW_KEY_S)) {
        m_Position -= m_Front * velocity;
    }
    
    // Left movement (A key)
    if (window.IsKeyPressed(GLFW_KEY_A)) {
        m_Position -= m_Right * velocity;
    }
    
    // Right movement (D key)
    if (window.IsKeyPressed(GLFW_KEY_D)) {
        m_Position += m_Right * velocity;
    }
}

glm::mat4 Camera::GetViewMatrix() const 
{
    return glm::lookAt(m_Position, m_Position + m_Front, m_Up);
}

glm::mat4 Camera::GetProjectionMatrix(float aspectRatio) const {
    return glm::perspective(glm::radians(m_Fov), aspectRatio, 0.1f, 100.0f);
}

glm::vec3 Camera::GetPosition() const {
    return m_Position;
}

glm::vec3 Camera::GetForward() const {
    return m_Front;
}

glm::vec3 Camera::GetRight() const {
    return m_Right;
}

glm::vec3 Camera::GetUp() const {
    return m_Up;
}

void Camera::ProcessMouseMovement(float xOffset, float yOffset) 
{
    // Scale the offsets by sensitivity
    xOffset *= m_MouseSensitivity;
    yOffset *= m_MouseSensitivity;
    
    // Update Euler angles
    m_Yaw += xOffset;
    m_Pitch += yOffset;
    
    // Clamp pitch to avoid camera flipping
    m_Pitch = std::clamp(m_Pitch, -89.0f, 89.0f);
    
    // Update camera vectors
    UpdateCameraVectors();
}

void Camera::UpdateCameraVectors() 
{
    // Calculate new front vector from Euler angles
    glm::vec3 front;
    front.x = cos(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
    front.y = sin(glm::radians(m_Pitch));
    front.z = sin(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
    m_Front = glm::normalize(front);
    
    // Recalculate right and up vectors
    m_Right = glm::normalize(glm::cross(m_Front, m_WorldUp));
    m_Up = glm::normalize(glm::cross(m_Right, m_Front));
} 