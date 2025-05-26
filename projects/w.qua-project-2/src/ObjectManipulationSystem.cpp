/**
 * @class ObjectManipulationSystem
 * @brief System for interactive manipulation of 3D objects.
 *
 * This system enables selection, translation, rotation, and scaling of 3D objects
 * through user interaction, supporting various manipulation modes and visual feedback.
 */

#include "ObjectManipulationSystem.hpp"
#include "Registry.hpp"
#include "Window.hpp"
#include "Systems.hpp"
#include "InputSystem.hpp"
#include "CollisionSystem.hpp"
#include "RenderSystem.hpp"
#include "CubeRenderer.hpp"
#include "SphereRenderer.hpp"
#include "PlaneRenderer.hpp"
#include "TriangleRenderer.hpp"
#include "RayRenderer.hpp"
#include "Intersection.hpp"
#include "Components.hpp"
#include "Primitives.hpp"
#include "Keybinds.hpp"

ObjectManipulationSystem::ObjectManipulationSystem(Registry& registry, Window& window)
    : m_Registry(registry), m_Window(window)
{
    Systems::g_InputSystem->SubscribeToMouseButton(Keybinds::MOUSE_BUTTON_LEFT, 
        [this](int button, int action, int mods)
        {
            // Only handle mouse clicks when ImGui is not capturing mouse
            if (ImGui::GetIO().WantCaptureMouse)
                return;
                
            // If camera is already in dragging mode, don't interfere
            if (Systems::g_InputSystem->IsMouseDragging())
                return;
                
            if (action == Keybinds::PRESS)
            {
                // Try to pick an object
                auto mousePos = Systems::g_InputSystem->GetMousePosition();
                Registry::Entity entity = PickObject(mousePos);
                
                if (entity != entt::null)
                {
                    // We found an entity to drag
                    StartDragging(entity);
                    
                    // Prevent camera from moving
                    Systems::g_InputSystem->StopDragging();
                }
            }
            else if (action == Keybinds::RELEASE)
            {
                // Stop dragging if we were dragging an entity
                if (IsDragging())
                {
                    StopDragging();
                }
            }
        });
        
    // Register mouse movement for dragging
    Systems::g_InputSystem->SubscribeToMouseMove(
        [this](double xpos, double ypos) 
        {
            // Only process if we're dragging an entity and ImGui isn't capturing mouse
            if (IsDragging() && !ImGui::GetIO().WantCaptureMouse)
            {
                glm::vec2 mousePos(xpos, ypos);
                DragSelected(mousePos);
            }
        });
}

ObjectManipulationSystem::~ObjectManipulationSystem()
{
    // No specific cleanup needed
}

void ObjectManipulationSystem::Update(float deltaTime)
{
    UpdateCollisionColors();
}

Registry::Entity ObjectManipulationSystem::PickObject(const glm::vec2& screenPos)
{
    // Convert screen position to a ray in world space
    Ray ray = ScreenToWorldRay(screenPos);
    
    auto view = m_Registry.View<TransformComponent, CollisionComponent>();
    
    Registry::Entity closestEntity = entt::null;
    float closestDistance = std::numeric_limits<float>::max();
    
    // Test ray against each entity's collision shape
    for (auto entity : view)
    {
        auto& collisionComp = m_Registry.GetComponent<CollisionComponent>(entity);
        
        // Skip entities with no shape
        if (collisionComp.m_ShapeType == CollisionShapeType::None)
            continue;
            
        float t = 0.0f;
        bool hit = false;
        
        // Test ray against collision shape based on shape type
        switch (collisionComp.m_ShapeType)
        {
            case CollisionShapeType::Sphere:
            {
                BoundingSphere sphere(collisionComp.m_Sphere.m_Center, collisionComp.m_Sphere.m_Radius);
                hit = Intersection::SphereVsRay(sphere, ray, t);
                break;
            }
            case CollisionShapeType::AABB:
            {
                AABB aabb(collisionComp.m_AABB.m_Center, collisionComp.m_AABB.m_HalfExtents);
                hit = Intersection::RayVsAABB(ray, aabb, t);
                break;
            }
            case CollisionShapeType::Triangle:
            {
                Triangle triangle(
                    Point3D(collisionComp.m_Triangle.m_Vertices[0]),
                    Point3D(collisionComp.m_Triangle.m_Vertices[1]),
                    Point3D(collisionComp.m_Triangle.m_Vertices[2]));
                hit = Intersection::RayVsTriangle(ray, triangle, t);
                break;
            }
            case CollisionShapeType::Plane:
            {
                Plane plane(
                    collisionComp.m_Plane.m_Normal.x,
                    collisionComp.m_Plane.m_Normal.y,
                    collisionComp.m_Plane.m_Normal.z,
                    collisionComp.m_Plane.m_Distance);
                hit = Intersection::RayVsPlane(ray, plane, t);
                break;
            }
            default:
                break;
        }
        
        // If we hit and it's closer than previous hits, update closest entity
        if (hit && t > 0.0f && t < closestDistance)
        {
            closestDistance = t;
            closestEntity = entity;
        }
    }
    
    return closestEntity;
}

void ObjectManipulationSystem::StartDragging(Registry::Entity entity)
{
    if (entity == entt::null)
        return;
        
    m_DraggingEntity = entity;
    
    if (m_Registry.HasComponent<RenderComponent>(entity))
    {
        auto& renderComp = m_Registry.GetComponent<RenderComponent>(entity);
        auto renderable = renderComp.m_Renderable;
        
        // Handle different renderable types for regular objects
        if (auto sphereRenderer = std::dynamic_pointer_cast<SphereRenderer>(renderable))
        {
            m_OriginalColors[entity] = sphereRenderer->GetColor();
            sphereRenderer->SetColor(SELECTED_COLOR);
        }
        else if (auto cubeRenderer = std::dynamic_pointer_cast<CubeRenderer>(renderable))
        {
            m_OriginalColors[entity] = cubeRenderer->GetColor();
            cubeRenderer->SetColor(SELECTED_COLOR);
        }
        else if (auto triangleRenderer = std::dynamic_pointer_cast<TriangleRenderer>(renderable))
        {
            m_OriginalColors[entity] = triangleRenderer->GetColor();
            triangleRenderer->SetColor(SELECTED_COLOR);
        }
        else if (auto planeRenderer = std::dynamic_pointer_cast<PlaneRenderer>(renderable))
        {
            m_OriginalColors[entity] = planeRenderer->GetColor();
            planeRenderer->SetColor(SELECTED_COLOR);
        }
        else if (auto rayRenderer = std::dynamic_pointer_cast<RayRenderer>(renderable))
        {
            m_OriginalColors[entity] = rayRenderer->GetColor();
            rayRenderer->SetColor(SELECTED_COLOR);
        }
    }
    
    // Set up the dragging plane
    if (m_Registry.HasComponent<TransformComponent>(entity))
    {
        auto& transform = m_Registry.GetComponent<TransformComponent>(entity);
        
        // Get camera position and view direction
        auto cameraView = m_Registry.View<CameraComponent>();
        if (!cameraView.empty())
        {
            auto cameraEntity = *cameraView.begin();
            auto& camera = m_Registry.GetComponent<CameraComponent>(cameraEntity);
            
            // Set the drag plane normal to be the camera view direction
            m_DragPlaneNormal = camera.m_FPS.m_CameraFront;
            
            // Set the drag plane point to be the entity's position
            m_DragPlanePoint = transform.m_Position;
            
            // Calculate the offset from the entity's position to the drag point
            glm::vec2 mousePos = Systems::g_InputSystem->GetMousePosition();
            glm::vec3 dragPos = GetDragPosition(mousePos);
            m_DragOffset = transform.m_Position - dragPos;
        }
    }
}

void ObjectManipulationSystem::StopDragging()
{
    if (m_DraggingEntity != entt::null)
    {
        // Restore original color
        ResetEntityColors(m_DraggingEntity);
        
        // Reset dragging state
        m_DraggingEntity = entt::null;
    }
}

void ObjectManipulationSystem::DragSelected(const glm::vec2& screenPos)
{
    if (m_DraggingEntity == entt::null)
        return;
        
    if (m_Registry.HasComponent<TransformComponent>(m_DraggingEntity))
    {
        // Get the drag position in world space
        glm::vec3 dragPos = GetDragPosition(screenPos);
        
        // Apply the offset to get the new entity position
        glm::vec3 newPosition = dragPos + m_DragOffset;
        
        // Update the entity's position
        auto& transform = m_Registry.GetComponent<TransformComponent>(m_DraggingEntity);
        transform.m_Position = newPosition;
        transform.UpdateModelMatrix();
        
        // Update collider position if entity has a collision component
        if (m_Registry.HasComponent<CollisionComponent>(m_DraggingEntity))
        {
            auto& collisionComp = m_Registry.GetComponent<CollisionComponent>(m_DraggingEntity);
            collisionComp.UpdateTransform(newPosition, transform.m_Scale);
        }
        
        // If this is the light source, update the lighting calculations
        if (IsLightSource(m_DraggingEntity))
        {
            Systems::g_RenderSystem->UpdateLightFromVisualization();
        }
    }
}

void ObjectManipulationSystem::UpdateCollisionColors()
{
    // Skip color updates if we're dragging an entity
    if (IsDragging())
        return;
        
    // Get all entities with render components AND collision components (exclude light visualization)
    auto renderView = m_Registry.View<RenderComponent, CollisionComponent>();
    std::vector<Registry::Entity> entities;
    
    // Filter out the light source from normal object color management
    for (auto entity : renderView) {
        if (!IsLightSource(entity)) {
            entities.push_back(entity);
        }
    }
    
    // Reset all entity colors to their base colors (excluding light source)
    int objectIndex = 0;
    for (auto entity : entities)
    {
        auto& renderComp = m_Registry.GetComponent<RenderComponent>(entity);
        auto renderable = renderComp.m_Renderable;
        
        // Determine base color for this object
        glm::vec3 baseColor;
        if (objectIndex == 0) 
        {
            baseColor = OBJECT1_COLOR; // Green for first object
        } else if (objectIndex == 1) 
        {
            baseColor = OBJECT2_COLOR; // Blue for second object
        } else 
        {
            // Use original color or a default for additional objects
            auto colorIt = m_OriginalColors.find(entity);
            if (colorIt != m_OriginalColors.end()) 
            {
                baseColor = colorIt->second;
            } 
            else 
            {
                baseColor = glm::vec3(0.5f, 0.5f, 0.5f); // Gray for additional objects
            }
        }
        
        // Store original color if not already stored
        if (m_OriginalColors.find(entity) == m_OriginalColors.end())
        {
            m_OriginalColors[entity] = baseColor;
        }
        
        // Set the base color for non-colliding state
        if (auto sphereRenderer = std::dynamic_pointer_cast<SphereRenderer>(renderable))
        {
            sphereRenderer->SetColor(baseColor);
        }
        else if (auto cubeRenderer = std::dynamic_pointer_cast<CubeRenderer>(renderable))
        {
            cubeRenderer->SetColor(baseColor);
        }
        else if (auto triangleRenderer = std::dynamic_pointer_cast<TriangleRenderer>(renderable))
        {
            triangleRenderer->SetColor(baseColor);
        }
        else if (auto planeRenderer = std::dynamic_pointer_cast<PlaneRenderer>(renderable))
        {
            planeRenderer->SetColor(baseColor);
        }
        else if (auto rayRenderer = std::dynamic_pointer_cast<RayRenderer>(renderable))
        {
            rayRenderer->SetColor(baseColor);
        }
        
        objectIndex++;
    }
    
    // Get collision pairs
    const auto& collisions = Systems::g_CollisionSystem->GetCollisions();
    
    // Set colliding entities to collision color (red)
    for (const auto& collision : collisions)
    {
        Registry::Entity entity1 = collision.entity1;
        Registry::Entity entity2 = collision.entity2;
        
        // Set color of entity1 if it has a render component
        if (m_Registry.HasComponent<RenderComponent>(entity1))
        {
            auto& renderComp = m_Registry.GetComponent<RenderComponent>(entity1);
            auto renderable = renderComp.m_Renderable;
            
            if (auto sphereRenderer = std::dynamic_pointer_cast<SphereRenderer>(renderable))
            {
                sphereRenderer->SetColor(COLLISION_COLOR);
            }
            else if (auto cubeRenderer = std::dynamic_pointer_cast<CubeRenderer>(renderable))
            {
                cubeRenderer->SetColor(COLLISION_COLOR);
            }
            else if (auto triangleRenderer = std::dynamic_pointer_cast<TriangleRenderer>(renderable))
            {
                triangleRenderer->SetColor(COLLISION_COLOR);
            }
            else if (auto planeRenderer = std::dynamic_pointer_cast<PlaneRenderer>(renderable))
            {
                planeRenderer->SetColor(COLLISION_COLOR);
            }
            else if (auto rayRenderer = std::dynamic_pointer_cast<RayRenderer>(renderable))
            {
                rayRenderer->SetColor(COLLISION_COLOR);
            }
        }
        
        // Set color of entity2 if it has a render component
        if (m_Registry.HasComponent<RenderComponent>(entity2))
        {
            auto& renderComp = m_Registry.GetComponent<RenderComponent>(entity2);
            auto renderable = renderComp.m_Renderable;
            
            if (auto sphereRenderer = std::dynamic_pointer_cast<SphereRenderer>(renderable))
            {
                sphereRenderer->SetColor(COLLISION_COLOR);
            }
            else if (auto cubeRenderer = std::dynamic_pointer_cast<CubeRenderer>(renderable))
            {
                cubeRenderer->SetColor(COLLISION_COLOR);
            }
            else if (auto triangleRenderer = std::dynamic_pointer_cast<TriangleRenderer>(renderable))
            {
                triangleRenderer->SetColor(COLLISION_COLOR);
            }
            else if (auto planeRenderer = std::dynamic_pointer_cast<PlaneRenderer>(renderable))
            {
                planeRenderer->SetColor(COLLISION_COLOR);
            }
            else if (auto rayRenderer = std::dynamic_pointer_cast<RayRenderer>(renderable))
            {
                rayRenderer->SetColor(COLLISION_COLOR);
            }
        }
    }
    
    // Ensure light source always stays yellow regardless of collisions
    Registry::Entity lightEntity = Systems::g_RenderSystem->GetLightVisualizationEntity();
    if (lightEntity != entt::null && m_Registry.HasComponent<RenderComponent>(lightEntity))
    {
        auto& renderComp = m_Registry.GetComponent<RenderComponent>(lightEntity);
        auto renderable = renderComp.m_Renderable;
        
        if (auto sphereRenderer = std::dynamic_pointer_cast<SphereRenderer>(renderable))
        {
            sphereRenderer->SetColor(LIGHT_COLOR);
        }
    }
}

Ray ObjectManipulationSystem::ScreenToWorldRay(const glm::vec2& screenPos)
{
    // Get camera information
    auto cameraView = m_Registry.View<CameraComponent>();
    if (cameraView.empty())
    {
        // Return a default ray if no camera is found
        return Ray(glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    }
    
    auto cameraEntity = *cameraView.begin();
    auto& camera = m_Registry.GetComponent<CameraComponent>(cameraEntity);
    
    // Calculate normalized device coordinates
    float x = (2.0f * screenPos.x) / m_Window.GetWidth() - 1.0f;
    float y = 1.0f - (2.0f * screenPos.y) / m_Window.GetHeight(); // Flip Y
    
    // Create NDC position
    glm::vec4 rayClip(x, y, -1.0f, 1.0f);
    
    // Transform to eye space
    float aspectRatio = static_cast<float>(m_Window.GetWidth()) / static_cast<float>(m_Window.GetHeight());
    glm::mat4 projMatrix = camera.GetProjectionMatrix(aspectRatio);
    glm::mat4 invProj = glm::inverse(projMatrix);
    glm::vec4 rayEye = invProj * rayClip;
    rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0f, 0.0f);
    
    // Transform to world space
    glm::mat4 viewMatrix = camera.GetViewMatrix();
    glm::mat4 invView = glm::inverse(viewMatrix);
    glm::vec4 rayWorld = invView * rayEye;
    
    // Normalize direction
    glm::vec3 rayDirection = glm::normalize(glm::vec3(rayWorld));
    
    // Create and return the ray
    return Ray(camera.GetPosition(), rayDirection);
}

void ObjectManipulationSystem::ResetEntityColors(Registry::Entity entity)
{
    if (entity == entt::null)
        return;
        
    // Restore original color if available
    auto colorIt = m_OriginalColors.find(entity);
    if (colorIt != m_OriginalColors.end())
    {
        if (m_Registry.HasComponent<RenderComponent>(entity))
        {
            auto& renderComp = m_Registry.GetComponent<RenderComponent>(entity);
            auto renderable = renderComp.m_Renderable;
            
            if (auto sphereRenderer = std::dynamic_pointer_cast<SphereRenderer>(renderable))
            {
                sphereRenderer->SetColor(colorIt->second);
            }
            else if (auto cubeRenderer = std::dynamic_pointer_cast<CubeRenderer>(renderable))
            {
                cubeRenderer->SetColor(colorIt->second);
            }
            else if (auto triangleRenderer = std::dynamic_pointer_cast<TriangleRenderer>(renderable))
            {
                triangleRenderer->SetColor(colorIt->second);
            }
            else if (auto planeRenderer = std::dynamic_pointer_cast<PlaneRenderer>(renderable))
            {
                planeRenderer->SetColor(colorIt->second);
            }
            else if (auto rayRenderer = std::dynamic_pointer_cast<RayRenderer>(renderable))
            {
                rayRenderer->SetColor(colorIt->second);
            }
        }
    }
}

glm::vec3 ObjectManipulationSystem::GetDragPosition(const glm::vec2& screenPos)
{
    // Convert screen position to ray
    Ray ray = ScreenToWorldRay(screenPos);
    
    // Intersect ray with drag plane
    Plane dragPlane = Plane::FromNormalAndPoint(m_DragPlaneNormal, m_DragPlanePoint);
    
    float t = 0.0f;
    if (Intersection::RayVsPlane(ray, dragPlane, t) && t > 0.0f)
    {
        // Return the intersection point
        return ray.GetPoint(t);
    }
    
    // Fallback - return the current entity position
    if (m_DraggingEntity != entt::null && m_Registry.HasComponent<TransformComponent>(m_DraggingEntity))
    {
        return m_Registry.GetComponent<TransformComponent>(m_DraggingEntity).m_Position;
    }
    
    // Last resort fallback
    return glm::vec3(0.0f);
}

bool ObjectManipulationSystem::IsLightSource(Registry::Entity entity) const
{
    // Get the light visualization entity from the render system
    return entity == Systems::g_RenderSystem->GetLightVisualizationEntity();
} 