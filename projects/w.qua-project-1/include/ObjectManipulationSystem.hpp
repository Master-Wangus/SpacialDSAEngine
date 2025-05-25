#pragma once

#include "pch.h"
#include "Registry.hpp"
#include "Window.hpp"
#include "Components.hpp"
#include "Primitives.hpp"

// Forward declarations
class InputSystem;
class CollisionSystem;

class ObjectManipulationSystem
{
public:
    ObjectManipulationSystem(Registry& registry, Window& window);
    ~ObjectManipulationSystem();

    void Update(float deltaTime);
    
    // Ray casting for object selection
    Registry::Entity PickObject(const glm::vec2& screenPos);
    
    // Manipulation methods
    void StartDragging(Registry::Entity entity);
    void StopDragging();
    void DragSelected(const glm::vec2& screenDelta);
    bool IsDragging() const { return m_DraggingEntity != entt::null; }
    
    // Collision visualization
    void UpdateCollisionColors();
    
private:
    // Convert screen position to world ray
    Ray ScreenToWorldRay(const glm::vec2& screenPos);
    
    // Reset entity colors to their original state
    void ResetEntityColors(Registry::Entity entity);
    
    // Check if entity is the light source visualization
    bool IsLightSource(Registry::Entity entity) const;
    
    // Calculate drag plane and position
    glm::vec3 GetDragPosition(const glm::vec2& screenPos);
    
    Registry& m_Registry;
    Window& m_Window;
    Registry::Entity m_DraggingEntity = entt::null;
    
    // State variables for dragging
    glm::vec3 m_DragPlaneNormal;
    glm::vec3 m_DragPlanePoint;
    glm::vec3 m_DragOffset;
    
    // Original colors of entities (for restoring)
    std::unordered_map<Registry::Entity, glm::vec3> m_OriginalColors;
    
    // Constants for visualization
    const glm::vec3 COLLISION_COLOR = glm::vec3(1.0f, 0.0f, 0.0f); // Red
    const glm::vec3 OBJECT1_COLOR = glm::vec3(0.0f, 1.0f, 0.0f);   // Green
    const glm::vec3 OBJECT2_COLOR = glm::vec3(0.0f, 0.0f, 1.0f);   // Blue
    const glm::vec3 SELECTED_COLOR = glm::vec3(1.0f, 1.0f, 0.0f);  // Yellow
    const glm::vec3 LIGHT_COLOR = glm::vec3(1.0f, 1.0f, 0.0f);     // Yellow for light source
}; 