#pragma once

#include "pch.h"
#include "Components.hpp"
#include "EventSystem.hpp"
#include "Keybinds.hpp"

// Forward declarations
class Window;
class Registry;

struct Ray
{
    glm::vec3 origin;
    glm::vec3 direction;
};


class PickingSystem
{
public:
    /**
     * @brief Constructs a PickingSystem with references to the ECS registry and application window.
     * @param registry Reference to the ECS registry
     * @param window   Reference to the application window
     */
    PickingSystem(Registry& registry, Window& window);

    /**
     * @brief Casts a ray based on the provided screen coordinates and returns the entity that is
     *        first intersected (closest to the camera).
     * @param screenPos Screen position in pixel coordinates (origin at top-left)
     * @return The intersected entity, or entt::null if no entity was hit.
     */
    Registry::Entity Pick(const glm::vec2& screenPos);

private:
    /**
     * @brief Converts screen coordinates to a world-space ray using the currently active camera.
     * @param screenPos Screen position in pixels
     * @return Generated world-space ray
     */
    Ray ScreenToWorldRay(const glm::vec2& screenPos);

    /**
     * @brief Tests if a ray intersects an AABB.
     * @param ray Ray to test
     * @param aabb Axis-aligned bounding box in world space
     * @param tHit Output param – distance from ray origin to intersection (only valid if function returns true)
     * @return True if the ray intersects the AABB, false otherwise
     */
    bool RayIntersectsAABB(const Ray& ray, const Aabb& aabb, float& tHit) const;

    // Dragging helpers
    glm::vec3 GetIntersectionPointOnDragPlane(const Ray& ray) const;

    // Event handlers for full interaction
    void HandleMouseButtonRelease(const EventData& eventData);
    void HandleMouseMove(const EventData& eventData);

    // Event handler for mouse button presses
    void HandleMouseButtonPress(const EventData& eventData);

    // Selection highlighting helpers
    void HighlightEntity(Registry::Entity entity);
    void ResetEntityHighlight(Registry::Entity entity);

    // Drag state
    Registry::Entity m_DraggingEntity = entt::null;
    glm::vec3 m_DragPlaneNormal{};
    glm::vec3 m_DragPlanePoint{};
    glm::vec3 m_DragOffset{};

    Registry& m_Registry;
    Window& m_Window;

    Registry::Entity m_SelectedEntity = entt::null;
    std::unordered_map<Registry::Entity, Material> m_OriginalMaterials;

    const glm::vec3 SELECTED_COLOR = glm::vec3(1.0f, 1.0f, 0.0f); // Yellow highlight
}; 