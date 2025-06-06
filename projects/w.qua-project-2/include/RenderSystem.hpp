/**
 * @class RenderSystem
 * @brief System for rendering 3D objects and scenes.
 *
 * This system manages the rendering process of 3D objects in the scene,
 * including camera view, projection setup, and proper draw call ordering.
 */

#pragma once

#include "pch.h"

class Shader;
class Window;
class CameraSystem;

struct RenderComponent;
struct TransformComponent;
struct CameraComponent;
struct DirectionalLightComponent;
struct BoundingComponent;
struct Aabb;
struct Sphere;

class RenderSystem 
{
public:
    RenderSystem(Registry& registry, Window& window, const std::shared_ptr<Shader>& shader);
    
    void Initialize();
    void Render();
    void Shutdown();
    
    // Get the shader used by the render system
    std::shared_ptr<Shader> GetShader() const { return m_Shader; }
    
    // Lighting control
    void ToggleDirectionalLight(bool enabled);
    bool IsDirectionalLightEnabled() const;
    void UpdateLightFromVisualization(); // Update light direction from visualization position
    Registry::Entity GetLightVisualizationEntity() const { return m_LightVisualizationEntity; }
    
    // Bounding volume visibility controls
    void SetShowAABB(bool show);
    void SetShowRitterSphere(bool show);
    void SetShowLarsonSphere(bool show);
    void SetShowPCASphere(bool show);
    void SetShowOBB(bool show);
    
    bool IsAABBVisible() const;
    bool IsRitterSphereVisible() const;
    bool IsLarsonSphereVisible() const;
    bool IsPCASphereVisible() const;
    bool IsOBBVisible() const;
    
    // Main object visibility controls
    void SetShowMainObjects(bool show);
    bool IsShowMainObjects() const;
    
    // Frustum culling controls
    void EnableFrustumCulling(bool enable);
    bool IsFrustumCullingEnabled() const;
    void SetCameraSystem(CameraSystem* cameraSystem);

private:
    void SetupLighting();
    void UpdateLighting();
    void SetupMaterial();
    void CreateLightSourceVisualization(const struct DirectionalLight& light);
    
    Registry& m_Registry;
    Window& m_Window;
    std::shared_ptr<Shader> m_Shader;
    Registry::Entity m_LightEntity = entt::null;
    Registry::Entity m_LightVisualizationEntity = entt::null;
    
    // Bounding volume visibility flags
    bool m_ShowAABB = true;
    bool m_ShowRitterSphere = true;
    bool m_ShowLarsonSphere = true;
    bool m_ShowPCASphere = true;
    bool m_ShowOBB = true;
    
    // Main object display control
    bool m_ShowMainObjects = true;
    
    // Frustum culling control
    bool m_EnableFrustumCulling = false;
    CameraSystem* m_CameraSystem = nullptr;
}; 