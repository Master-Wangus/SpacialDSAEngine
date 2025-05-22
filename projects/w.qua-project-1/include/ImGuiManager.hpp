#pragma once

#include "Window.hpp"
#include "Systems.hpp"

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
    void RenderSceneSelector(Registry& registry);
    void RenderLightingControls(Registry& registry);
    void RenderStats();
    
    // Helper functions
    void HelpMarker(const char* desc);
    void SwitchScene(Registry& registry, DemoSceneType sceneType);

private:
    Window& m_Window;
    bool m_Initialized = false;
    float m_LastFrameTime = 0.0f;
    int m_FrameCount = 0;
    float m_FrameRate = 0.0f;
    float m_FrameTimeAccumulator = 0.0f;
}; 