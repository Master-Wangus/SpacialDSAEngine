#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Window.hpp"

/**
 * @class Camera
 * @brief FPS-style camera with movement and rotation
 */
class Camera {
public:
    /**
     * @brief Constructor that initializes camera with given position and orientation
     * @param position Initial position
     * @param worldUp Up vector in world space
     */
    Camera(const glm::vec3& position = glm::vec3(0.0f, 0.0f, 3.0f), const glm::vec3& worldUp = glm::vec3(0.0f, 1.0f, 0.0f));
    
    /**
     * @brief Updates camera based on keyboard and mouse input
     * @param window Window object for input
     * @param deltaTime Time since last frame
     */
    void update(const Window& window, float deltaTime);
    
    /**
     * @brief Gets the view matrix for this camera
     * @return The view matrix
     */
    glm::mat4 getViewMatrix() const;
    
    /**
     * @brief Gets the projection matrix for this camera
     * @param aspectRatio Window aspect ratio (width / height)
     * @return The projection matrix
     */
    glm::mat4 getProjectionMatrix(float aspectRatio) const;
    
    /**
     * @brief Gets the camera's position in world space
     * @return Position vector
     */
    glm::vec3 getPosition() const;
    
    /**
     * @brief Gets the camera's forward direction
     * @return Forward vector
     */
    glm::vec3 getForward() const;
    
    /**
     * @brief Gets the camera's right direction
     * @return Right vector
     */
    glm::vec3 getRight() const;
    
    /**
     * @brief Gets the camera's up direction
     * @return Up vector
     */
    glm::vec3 getUp() const;
    
    /**
     * @brief Process mouse movement for camera rotation
     * @param xOffset X movement of the mouse
     * @param yOffset Y movement of the mouse
     */
    void processMouseMovement(float xOffset, float yOffset);

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
    
    /**
     * @brief Updates camera vectors based on Euler angles
     */
    void updateCameraVectors();
}; 