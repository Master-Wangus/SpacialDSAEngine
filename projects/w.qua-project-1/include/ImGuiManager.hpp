#pragma once

#include "pch.h"
#include "Window.hpp"

// Forward declarations
class Registry;
class Shader;

class ImGuiManager 
{
public:
    ImGuiManager(Window& window);
    ~ImGuiManager();

    void Init();
    void NewFrame();
    void Render();
    void Shutdown();

    // Rendering various ImGui windows
    void RenderMainWindow(Registry& registry);
    void RenderCollisionInfo(Registry& registry);
    void RenderEntityList(Registry& registry);
    void RenderCameraSettings(Registry& registry);
    void RenderStats();

private:
    Window& m_Window;
    bool m_Initialized = false;
    float m_LastFrameTime = 0.0f;
    int m_FrameCount = 0;
    float m_FrameRate = 0.0f;
    float m_FrameTimeAccumulator = 0.0f;
}; 