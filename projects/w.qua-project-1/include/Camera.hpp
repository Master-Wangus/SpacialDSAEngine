#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Window.hpp"

class Camera 
{
public:
    Camera(const glm::vec3& position = glm::vec3(0.0f, 0.0f, 3.0f), const glm::vec3& worldUp = glm::vec3(0.0f, 1.0f, 0.0f));
    virtual ~Camera() = default;
    virtual void Update(const Window& window, float deltaTime);
    virtual glm::mat4 GetViewMatrix() const;
    virtual glm::mat4 GetProjectionMatrix(float aspectRatio) const;
    virtual glm::vec3 GetPosition() const;
    virtual glm::vec3 GetForward() const;
    virtual glm::vec3 GetRight() const;
    virtual glm::vec3 GetUp() const;
    virtual void ProcessMouseMovement(float xOffset, float yOffset);

    // Getters
    float GetMovementSpeed() const { return m_MovementSpeed; }
    float GetMouseSensitivity() const { return m_MouseSensitivity; }
    float GetFov() const { return m_Fov; }
    float GetYaw() const { return m_Yaw; }
    float GetPitch() const { return m_Pitch; }

    // Setters
    void SetPosition(const glm::vec3& position) { m_Position = position; }
    void SetYaw(float yaw) { m_Yaw = yaw; }
    void SetPitch(float pitch) { m_Pitch = pitch; }

protected:
    void UpdateCameraVectors();

private:
    // Camera position and orientation
    glm::vec3 m_Position;
    glm::vec3 m_Front;
    glm::vec3 m_Up;
    glm::vec3 m_Right;
    glm::vec3 m_WorldUp;
    
    // Euler angles
    float m_Yaw;
    float m_Pitch;
    
    // Camera options
    float m_MovementSpeed;
    float m_MouseSensitivity;
    float m_Fov; // Field of view in degrees
    
    // Mouse position from last frame
    float m_LastX;
    float m_LastY;
    bool m_FirstMouse;
}; 