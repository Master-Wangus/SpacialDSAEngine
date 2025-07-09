#include "PickingSystem.hpp"
#include "Window.hpp"
#include "CameraSystem.hpp"
#include <limits>

// Global systems access (declared in Systems namespace)
#include "Systems.hpp"
#include "Keybinds.hpp"
#include "InputSystem.hpp"
#include "EventSystem.hpp"

using namespace Systems; // For accessing global systems

//------------------------------------------------------------------------------
// Helper – epsilon for float comparisons
//------------------------------------------------------------------------------
static constexpr float kRayTMin = 0.0f;
static constexpr float kRayTMaxDefault = std::numeric_limits<float>::max();

//------------------------------------------------------------------------------
// PickingSystem implementation
//------------------------------------------------------------------------------

PickingSystem::PickingSystem(Registry& registry, Window& window)
    : m_Registry(registry), m_Window(window)
{
    // Subscribe to mouse button presses through the EventSystem
    EventSystem::Get().SubscribeToEvent(EventType::MouseButtonPress, [this](const EventData& e){ HandleMouseButtonPress(e); });

    // Also listen for button release and mouse movement to enable dragging
    EventSystem::Get().SubscribeToEvent(EventType::MouseButtonRelease, [this](const EventData& e){ HandleMouseButtonRelease(e); });
    EventSystem::Get().SubscribeToEvent(EventType::MouseMove, [this](const EventData& e){ HandleMouseMove(e); });
}

//------------------------------------------------------------------------------
Registry::Entity PickingSystem::Pick(const glm::vec2& screenPos)
{
    // Convert screen coordinates to world ray
    Ray ray = ScreenToWorldRay(screenPos);

    float closestT = kRayTMaxDefault;
    Registry::Entity closestEntity = entt::null;

    // Iterate through all entities that have both Transform and Bounding components
    auto view = m_Registry.View<TransformComponent, BoundingComponent>();

    for (auto entity : view)
    {
        auto& transform = view.get<TransformComponent>(entity);
        auto& bounds    = view.get<BoundingComponent>(entity);

        // Retrieve local-space AABB and transform it to world space
        const Aabb& localAabb = bounds.GetAABB();
        Aabb worldAabb        = localAabb;       // Make a copy so we can transform safely
        worldAabb.Transform(transform.m_Model);

        float tHit;
        if (RayIntersectsAABB(ray, worldAabb, tHit))
        {
            if (tHit < closestT)
            {
                closestT     = tHit;
                closestEntity = entity;
            }
        }
    }

    return closestEntity;
}

//------------------------------------------------------------------------------
Ray PickingSystem::ScreenToWorldRay(const glm::vec2& screenPos)
{
    Ray ray{};

    // Acquire the active camera – we use the first CameraComponent found.
    CameraComponent* activeCamera = nullptr;
    Registry::Entity cameraEntity = entt::null;

    for (auto entity : m_Registry.View<CameraComponent>())
    {
        cameraEntity = entity;
        activeCamera = &m_Registry.GetComponent<CameraComponent>(entity);
        break; // Use first camera encountered
    }

    if (!activeCamera)
    {
        // No camera found – return default ray (should not happen in normal operation)
        ray.origin    = glm::vec3(0.0f);
        ray.direction = glm::vec3(0.0f, 0.0f, -1.0f);
        return ray;
    }

    const float width  = static_cast<float>(m_Window.GetWidth());
    const float height = static_cast<float>(m_Window.GetHeight());

    // Convert from screen (top-left origin) to Normalized Device Coordinates (NDC)
    glm::vec2 ndc;
    ndc.x = (2.0f * screenPos.x) / width - 1.0f;
    ndc.y = 1.0f - (2.0f * screenPos.y) / height; // Flip Y (GL NDC is -1 at bottom)

    // Prepare clip-space coordinates
    glm::vec4 rayClip(ndc.x, ndc.y, -1.0f, 1.0f); // -1 at near plane

    // Retrieve projection & view matrices
    const float aspectRatio = width / height;
    glm::mat4 projection    = activeCamera->m_Projection.GetProjectionMatrix(aspectRatio);
    glm::mat4 view          = activeCamera->GetViewMatrix();

    glm::mat4 invProjection = glm::inverse(projection);
    glm::vec4 rayEye        = invProjection * rayClip;
    rayEye                  = glm::vec4(rayEye.x, rayEye.y, -1.0f, 0.0f); // We are now in eye space

    glm::mat4 invView       = glm::inverse(view);
    glm::vec4 rayWorld4     = invView * rayEye;
    glm::vec3 rayWorld      = glm::normalize(glm::vec3(rayWorld4));

    ray.origin    = activeCamera->GetPosition();
    ray.direction = rayWorld;

    return ray;
}

//------------------------------------------------------------------------------
// Slab intersection test – returns true if ray hits AABB. The intersection
// distance along the ray (t-value) is written into tHit if provided.
//------------------------------------------------------------------------------
bool PickingSystem::RayIntersectsAABB(const Ray& ray, const Aabb& aabb, float& tHit) const
{
    float tMin = kRayTMin;
    float tMax = kRayTMaxDefault;

    const glm::vec3& o = ray.origin;
    const glm::vec3& d = ray.direction;
    const glm::vec3& min = aabb.min;
    const glm::vec3& max = aabb.max;

    for (int i = 0; i < 3; ++i)
    {
        if (std::abs(d[i]) < std::numeric_limits<float>::epsilon())
        {
            // Ray is parallel to slab. No hit if origin not within slab
            if (o[i] < min[i] || o[i] > max[i])
                return false;
        }
        else
        {
            float invD = 1.0f / d[i];
            float t0   = (min[i] - o[i]) * invD;
            float t1   = (max[i] - o[i]) * invD;

            if (t0 > t1)
                std::swap(t0, t1);

            tMin = t0 > tMin ? t0 : tMin;
            tMax = t1 < tMax ? t1 : tMax;

            if (tMin > tMax)
                return false;
        }
    }

    tHit = tMin;
    return true;
}

//------------------------------------------------------------------------------
void PickingSystem::HandleMouseButtonPress(const EventData& eventData)
{
    int button = std::get<int>(eventData);
    if (button != Keybinds::MOUSE_BUTTON_LEFT)
        return; // Only react to left-click

    // Retrieve current mouse position using the InputSystem
    if (!Systems::GetInputSystem())
        return;

    glm::vec2 mousePos = Systems::GetInputSystem()->GetMousePosition();

    Registry::Entity picked = Pick(mousePos);

    // Reset previous selection highlight (if any and not the same)
    if (m_SelectedEntity != entt::null && m_SelectedEntity != picked)
    {
        ResetEntityHighlight(m_SelectedEntity);
    }

    m_SelectedEntity = picked;

    if (m_SelectedEntity != entt::null)
    {
        HighlightEntity(m_SelectedEntity);

        // Prepare for dragging
        m_DraggingEntity = m_SelectedEntity;

        // Compute the drag plane (plane perpendicular to camera view passing through entity position)
        CameraComponent* camera = nullptr;
        for (auto ent : m_Registry.View<CameraComponent>())
        {
            camera = &m_Registry.GetComponent<CameraComponent>(ent);
            break;
        }

        if (camera)
        {
            glm::vec3 camPos = camera->GetPosition();
        }
        // We'll compute normal by direction from camera to entity
        if (m_Registry.HasComponent<TransformComponent>(m_DraggingEntity))
        {
            auto& trans = m_Registry.GetComponent<TransformComponent>(m_DraggingEntity);
            m_DragPlanePoint = trans.m_Position;
        }

        // Compute normal from camera to plane point (so drag plane faces camera)
        if (camera)
        {
            glm::vec3 camPos = camera->GetPosition();
            m_DragPlaneNormal = glm::normalize(camPos - m_DragPlanePoint);
        }
        else
        {
            m_DragPlaneNormal = glm::vec3(0.0f, 0.0f, 1.0f);
        }

        // Calculate initial drag offset
        Ray ray = ScreenToWorldRay(mousePos);
        glm::vec3 intersect = GetIntersectionPointOnDragPlane(ray);
        if (m_Registry.HasComponent<TransformComponent>(m_DraggingEntity))
        {
            auto& trans = m_Registry.GetComponent<TransformComponent>(m_DraggingEntity);
            m_DragOffset = trans.m_Position - intersect;
        }

        if (Systems::GetInputSystem())
            Systems::GetInputSystem()->StartDragging();
    }
}

//------------------------------------------------------------------------------
void PickingSystem::HighlightEntity(Registry::Entity entity)
{
    if (!m_Registry.HasComponent<RenderComponent>(entity))
        return;

    auto& renderComp = m_Registry.GetComponent<RenderComponent>(entity);
    if (!renderComp.m_Renderable)
        return;

    // Save original material if not already stored
    if (m_OriginalMaterials.find(entity) == m_OriginalMaterials.end())
    {
        m_OriginalMaterials[entity] = renderComp.m_Renderable->GetMaterial();
    }

    Material newMat = renderComp.m_Renderable->GetMaterial();
    newMat.m_DiffuseColor  = SELECTED_COLOR;
    newMat.m_AmbientColor  = SELECTED_COLOR;
    renderComp.m_Renderable->SetMaterial(newMat);
}

//------------------------------------------------------------------------------
void PickingSystem::ResetEntityHighlight(Registry::Entity entity)
{
    if (!m_Registry.HasComponent<RenderComponent>(entity))
        return;

    auto it = m_OriginalMaterials.find(entity);
    if (it == m_OriginalMaterials.end())
        return;

    auto& renderComp = m_Registry.GetComponent<RenderComponent>(entity);
    if (renderComp.m_Renderable)
    {
        renderComp.m_Renderable->SetMaterial(it->second);
    }

    m_OriginalMaterials.erase(it);
}

//------------------------------------------------------------------------------
glm::vec3 PickingSystem::GetIntersectionPointOnDragPlane(const Ray& ray) const
{
    // Compute intersection of ray with plane defined by m_DragPlanePoint and m_DragPlaneNormal
    float denom = glm::dot(ray.direction, m_DragPlaneNormal);
    if (std::abs(denom) < 1e-6f)
    {
        // Parallel, return origin projection
        return ray.origin;
    }

    float t = glm::dot(m_DragPlanePoint - ray.origin, m_DragPlaneNormal) / denom;
    return ray.origin + t * ray.direction;
}

//------------------------------------------------------------------------------
void PickingSystem::HandleMouseButtonRelease(const EventData& eventData)
{
    int button = std::get<int>(eventData);
    if (button != Keybinds::MOUSE_BUTTON_LEFT)
        return;

    if (m_DraggingEntity != entt::null)
    {
        m_DraggingEntity = entt::null;
        if (Systems::GetInputSystem())
            Systems::GetInputSystem()->StopDragging();
    }
}

//------------------------------------------------------------------------------
void PickingSystem::HandleMouseMove(const EventData& eventData)
{
    if (m_DraggingEntity == entt::null)
        return;

    glm::vec2 screenPos = std::get<glm::vec2>(eventData);

    Ray ray = ScreenToWorldRay(screenPos);

    glm::vec3 worldPos = GetIntersectionPointOnDragPlane(ray) + m_DragOffset;

    if (m_Registry.HasComponent<TransformComponent>(m_DraggingEntity))
    {
        auto& transform = m_Registry.GetComponent<TransformComponent>(m_DraggingEntity);
        transform.m_Position = worldPos;
        transform.UpdateModelMatrix();

        // Notify systems of transform change for this entity
        EventSystem::Get().FireEvent(EventType::TransformChanged, m_DraggingEntity);
    }
} 