#include "../include/Camera.hpp"
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <cmath>

// Constructor
Camera::Camera(const glm::vec3& position, const glm::vec3& worldUp)
    : m_position(position),
      m_worldUp(worldUp),
      m_yaw(-90.0f),        // Default yaw is -90 degrees (looking forward)
      m_pitch(0.0f),        // Default pitch is 0 degrees (looking horizontally)
      m_movementSpeed(2.5f),
      m_mouseSensitivity(0.1f),
      m_fov(45.0f),
      m_lastX(0.0f),
      m_lastY(0.0f),
      m_firstMouse(true)
{
    // Initialize camera vectors
    updateCameraVectors();
}

void Camera::update(const Window& window, float deltaTime) 
{
    // Handle keyboard input for camera movement
    float velocity = m_movementSpeed * deltaTime;
    
    // Forward movement (W key)
    if (window.isKeyPressed(GLFW_KEY_W)) {
        m_position += m_front * velocity;
    }
    
    // Backward movement (S key)
    if (window.isKeyPressed(GLFW_KEY_S)) {
        m_position -= m_front * velocity;
    }
    
    // Left movement (A key)
    if (window.isKeyPressed(GLFW_KEY_A)) {
        m_position -= m_right * velocity;
    }
    
    // Right movement (D key)
    if (window.isKeyPressed(GLFW_KEY_D)) {
        m_position += m_right * velocity;
    }
}

glm::mat4 Camera::getViewMatrix() const 
{
    return glm::lookAt(m_position, m_position + m_front, m_up);
}

glm::mat4 Camera::getProjectionMatrix(float aspectRatio) const {
    return glm::perspective(glm::radians(m_fov), aspectRatio, 0.1f, 100.0f);
}

glm::vec3 Camera::getPosition() const {
    return m_position;
}

glm::vec3 Camera::getForward() const {
    return m_front;
}

glm::vec3 Camera::getRight() const {
    return m_right;
}

glm::vec3 Camera::getUp() const {
    return m_up;
}

void Camera::processMouseMovement(float xOffset, float yOffset) 
{
    // Scale the offsets by sensitivity
    xOffset *= m_mouseSensitivity;
    yOffset *= m_mouseSensitivity;
    
    // Update Euler angles
    m_yaw += xOffset;
    m_pitch += yOffset;
    
    // Clamp pitch to avoid camera flipping
    m_pitch = std::clamp(m_pitch, -89.0f, 89.0f);
    
    // Update camera vectors
    updateCameraVectors();
}

void Camera::updateCameraVectors() 
{
    // Calculate new front vector from Euler angles
    glm::vec3 front;
    front.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    front.y = sin(glm::radians(m_pitch));
    front.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    m_front = glm::normalize(front);
    
    // Recalculate right and up vectors
    m_right = glm::normalize(glm::cross(m_front, m_worldUp));
    m_up = glm::normalize(glm::cross(m_right, m_front));
} 