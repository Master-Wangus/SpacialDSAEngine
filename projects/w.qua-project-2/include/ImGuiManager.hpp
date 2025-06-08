/**
 * @class ImGuiManager
 * @brief Manager for Dear ImGui integration and UI rendering.
 *
 * This class handles the initialization, rendering, and shutdown of the Dear ImGui
 * library for creating interactive debug interfaces and control panels.
 */

#pragma once
#include "pch.h"

// Forward declarations
class Registry;
class Shader;
class Window;
enum class DemoSceneType;

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
    void RenderCameraControls(Registry& registry);
    void RenderBoundingVolumeControls(Registry& registry);
    void RenderObjectVisibilityControls(Registry& registry);
    void RenderModelSelectionControls(Registry& registry);
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
    
    void UpdateFrameRate(float deltaTime);
}; 