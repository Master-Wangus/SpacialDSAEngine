#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Window.hpp"

class Camera 
{
public:
    Camera(const glm::vec3& position = glm::vec3(0.0f, 0.0f, 3.0f), const glm::vec3& worldUp = glm::vec3(0.0f, 1.0f, 0.0f));
    virtual ~Camera() = default;
    virtual void update(const Window& window, float deltaTime);
    virtual glm::mat4 getViewMatrix() const;
    virtual glm::mat4 getProjectionMatrix(float aspectRatio) const;
    virtual glm::vec3 getPosition() const;
    virtual glm::vec3 getForward() const;
    virtual glm::vec3 getRight() const;
    virtual glm::vec3 getUp() const;
    virtual void processMouseMovement(float xOffset, float yOffset);

private:
    // Camera position and orientation
    glm::vec3 m_position;
    glm::vec3 m_front;
    glm::vec3 m_up;
    glm::vec3 m_right;
    glm::vec3 m_worldUp;
    
    // Euler angles
    float m_yaw;
    float m_pitch;
    
    // Camera options
    float m_movementSpeed;
    float m_mouseSensitivity;
    float m_fov; // Field of view in degrees
    
    // Mouse position from last frame
    float m_lastX;
    float m_lastY;
    bool m_firstMouse;
    
    void updateCameraVectors();
}; 