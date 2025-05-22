#pragma once

#include "pch.h"
#include "Components.hpp"
#include "Registry.hpp"
#include "Window.hpp"
#include "FPSCameraSystem.hpp"

class Shader;

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

private:
    void SetupLighting();
    void UpdateLighting();
    void SetupMaterial();
    
    Registry& m_Registry;
    Window& m_Window;
    std::shared_ptr<Shader> m_Shader;
    Registry::Entity m_LightEntity = entt::null;
}; 