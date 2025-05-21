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

private:
    void SetupLighting();
    
    Registry& m_Registry;
    Window& m_Window;
    std::shared_ptr<Shader> m_Shader;
}; 