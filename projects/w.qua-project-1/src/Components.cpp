#include "../include/Components.hpp"
#include <glm/gtc/matrix_transform.hpp>

void TransformComponent::UpdateModelMatrix()
{
    m_Model = glm::mat4(1.0f);
    m_Model = glm::translate(m_Model, m_Position);
    m_Model = glm::rotate(m_Model, glm::radians(m_Rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
    m_Model = glm::rotate(m_Model, glm::radians(m_Rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    m_Model = glm::rotate(m_Model, glm::radians(m_Rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
    m_Model = glm::scale(m_Model, m_Scale);
} 