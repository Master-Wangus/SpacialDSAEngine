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
    /**
     * @brief Constructs the ImGui manager with a window reference.
     * @param window Window to integrate ImGui with
     */
    ImGuiManager(Window& window);
    
    /**
     * @brief Destructor for the ImGui manager.
     */
    ~ImGuiManager();

    /**
     * @brief Initializes the ImGui system.
     */
    void Init();
    
    /**
     * @brief Begins a new ImGui frame.
     */
    void NewFrame();
    
    /**
     * @brief Renders all ImGui elements.
     */
    void Render();
    
    /**
     * @brief Shuts down the ImGui system.
     */
    void Shutdown();

    // Rendering various ImGui windows
    /**
     * @brief Renders the main control window.
     * @param registry Entity registry reference
     */
    void RenderMainWindow(Registry& registry);
    
    /**
     * @brief Renders the scene selection window.
     * @param registry Entity registry reference
     */
    void RenderSceneSelector(Registry& registry);
    
    /**
     * @brief Renders lighting control interface.
     * @param registry Entity registry reference
     */
    void RenderLightingControls(Registry& registry);
    
    /**
     * @brief Renders camera control interface.
     * @param registry Entity registry reference
     */
    void RenderCameraControls(Registry& registry);
    
    /**
     * @brief Renders bounding volume visibility controls.
     * @param registry Entity registry reference
     */
    void RenderBoundingVolumeControls(Registry& registry);
    
    /**
     * @brief Renders wireframe rendering controls.
     * @param registry Entity registry reference
     */
    void RenderWireframeControls(Registry& registry);
    
    /**
     * @brief Renders object visibility controls.
     * @param registry Entity registry reference
     */
    void RenderObjectVisibilityControls(Registry& registry);
    
    /**
     * @brief Renders model selection controls.
     * @param registry Entity registry reference
     */
    void RenderModelSelectionControls(Registry& registry);
    
    /**
     * @brief Renders frustum culling controls.
     * @param registry Entity registry reference
     */
    void RenderFrustumControls(Registry& registry);
    
    /**
     * @brief Renders performance statistics.
     */
    void RenderStats();

    /**
     * @brief Renders per-model scaling controls.
     * @param registry Entity registry reference
     */
    void RenderScalingControls(Registry& registry);

    /**
     * @brief Renders BVH construction and visualisation controls.
     * @param registry Entity registry reference
     */
    void RenderBVHControls(Registry& registry);

private:
    Window& m_Window;
    bool m_Initialized = false;
    float m_LastFrameTime = 0.0f;
    int m_FrameCount = 0;
    float m_FrameRate = 0.0f;
    float m_FrameTimeAccumulator = 0.0f;
    
    /**
     * @brief Updates frame rate calculation.
     * @param deltaTime Time elapsed since last frame
     */
    void UpdateFrameRate(float deltaTime);
}; 