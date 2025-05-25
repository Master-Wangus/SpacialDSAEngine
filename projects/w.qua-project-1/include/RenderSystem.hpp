/**
 * @class RenderSystem
 * @brief System for rendering 3D objects and scenes.
 *
 * This system manages the rendering process of 3D objects in the scene,
 * including camera view, projection setup, and proper draw call ordering.
 */

#pragma once

#include "pch.h"

// Forward declarations
class Shader;
class Window;

// Forward declare components and registry
struct RenderComponent;
struct TransformComponent;
struct CameraComponent;
struct DirectionalLightComponent;

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
}; 