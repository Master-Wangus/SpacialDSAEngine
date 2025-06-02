/**
 * @class ImGuiManager
 * @brief Manager for Dear ImGui integration and UI rendering.
 *
 * This class handles the initialization, rendering, and shutdown of the Dear ImGui
 * library for creating interactive debug interfaces and control panels.
 */

#include "pch.h"
#include "ImGuiManager.hpp"
#include "Registry.hpp"
#include "Components.hpp"
#include "Systems.hpp"
#include "Shader.hpp"
#include "RenderSystem.hpp"
#include "Buffer.hpp"
#include "DemoScene.hpp"
#include "Window.hpp"

ImGuiManager::ImGuiManager(Window& window)
    : m_Window(window)
{
}

ImGuiManager::~ImGuiManager()
{
    Shutdown();
}

void ImGuiManager::Init()
{
    if (m_Initialized) return;

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    
    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    
    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL((GLFWwindow*)m_Window.GetNativeWindow(), true);
    ImGui_ImplOpenGL3_Init("#version 460");
    
    m_Initialized = true;
}

void ImGuiManager::NewFrame()
{
    if (!m_Initialized) return;
    
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void ImGuiManager::Render()
{
    if (!m_Initialized) return;
    
    // Rendering
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void ImGuiManager::Shutdown()
{
    if (!m_Initialized) return;
    
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    
    m_Initialized = false;
}

void ImGuiManager::RenderMainWindow(Registry& registry)
{
    ImGui::Begin("Geometry Toolbox Controls");
    
    // FPS counter and performance stats
    float currentTime = static_cast<float>(m_Window.GetTime());
    float deltaTime = currentTime - m_LastFrameTime;
    m_LastFrameTime = currentTime;
    
    m_FrameCount++;
    m_FrameTimeAccumulator += deltaTime;
    
    if (m_FrameTimeAccumulator >= 1.0f) 
    {
        m_FrameRate = static_cast<float>(m_FrameCount) / m_FrameTimeAccumulator;
        m_FrameCount = 0;
        m_FrameTimeAccumulator = 0.0f;
    }
    
    ImGui::Text("FPS: %.1f", m_FrameRate);
    ImGui::Text("Frame time: %.3f ms", deltaTime * 1000.0f);
    
    ImGui::Separator();
    
    // Demo scene selection
    if (ImGui::CollapsingHeader("Demo Scenes", ImGuiTreeNodeFlags_DefaultOpen))
    {
        RenderSceneSelector(registry);
    }
    
    // Camera controls
    if (ImGui::CollapsingHeader("Camera Controls"))
    {
        RenderCameraControls(registry);
    }
    
    // Lighting settings
    if (ImGui::CollapsingHeader("Lighting Settings"))
    {
        RenderLightingControls(registry);
    }
    
    // Debug statistics
    if (ImGui::CollapsingHeader("Debug Statistics")) 
    {
        RenderStats();
    }
    
    ImGui::End();
}

void ImGuiManager::RenderSceneSelector(Registry& registry)
{
    static const char* currentScene = "Demo Scene with Bounding Volumes";
    static int selectedScene = static_cast<int>(Systems::g_CurrentDemoScene);
    
    static const char* sceneNames[] = 
    {
        "Demo Scene",
    };
    
    ImGui::Text("Current Scene: %s", sceneNames[selectedScene]);
    ImGui::Separator();
   
}

void ImGuiManager::SwitchScene(Registry& registry, DemoSceneType sceneType)
{
    Systems::g_CurrentDemoScene = sceneType;
}

void ImGuiManager::RenderCameraControls(Registry& registry)
{
    auto cameraView = registry.View<CameraComponent>();
    if (cameraView.empty()) {
        ImGui::Text("No camera found");
        return;
    }
    
    auto cameraEntity = *cameraView.begin();
    auto& camera = registry.GetComponent<CameraComponent>(cameraEntity);
    
    const char* currentCameraType = (camera.m_ActiveCameraType == CameraType::FPS) ? "FPS Camera" : "Orbital Camera";
    ImGui::Text("Current Camera: %s", currentCameraType);
    
    glm::vec3 cameraPos = camera.GetPosition();
    ImGui::Text("Position: (%.2f, %.2f, %.2f)", cameraPos.x, cameraPos.y, cameraPos.z);
    
    ImGui::Separator();
    
    // Camera type specific information
    if (camera.m_ActiveCameraType == CameraType::FPS) 
    {
        ImGui::TextColored(ImVec4(0.5f, 1.0f, 0.5f, 1.0f), "FPS Camera Controls:");
        ImGui::Text("WASD - Move camera");
        ImGui::Text("Mouse + Right Click - Look around");
        ImGui::Text("C - Switch to Orbital camera");
        
        ImGui::Separator();
        
        // FPS Camera details
        ImGui::Text("Yaw: %.1f°", camera.m_FPS.m_YawAngle);
        ImGui::Text("Pitch: %.1f°", camera.m_FPS.m_PitchAngle);
        ImGui::Text("Speed: %.2f", camera.m_FPS.m_MovementSpeed);
    }
    else if (camera.m_ActiveCameraType == CameraType::Orbital) 
    {
        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.3f, 1.0f), "Orbital Camera Controls:");
        ImGui::Text("Mouse + Right Click - Orbit around target");
        ImGui::Text("Mouse Wheel - Zoom in/out");
        ImGui::Text("WASD - Move target point");
        ImGui::Text("C - Switch to FPS camera");
        
        ImGui::Separator();
        
        // Orbital Camera details
        glm::vec3 target = camera.m_Orbital.m_Target;
        ImGui::Text("Target: (%.2f, %.2f, %.2f)", target.x, target.y, target.z);
        ImGui::Text("Distance: %.2f", camera.m_Orbital.m_Distance);
        ImGui::Text("Yaw: %.1f°", camera.m_Orbital.m_Yaw);
        ImGui::Text("Pitch: %.1f°", camera.m_Orbital.m_Pitch);
        ImGui::Text("Zoom Range: %.1f - %.1f", camera.m_Orbital.m_MinDistance, camera.m_Orbital.m_MaxDistance);
    }
}

void ImGuiManager::RenderLightingControls(Registry& registry)
{
    static GLuint materialUBO = 1; // Index 1 as set in the SetupMaterial function
    static float ambientIntensity = 0.5f;
    static float ambientColor[3] = {1.0f, 1.0f, 1.0f};
    
    if (Systems::g_RenderSystem)
    {
        bool lightEnabled = Systems::g_RenderSystem->IsDirectionalLightEnabled();
        
        if (ImGui::Checkbox("Enable Directional Lighting", &lightEnabled))
        {
            Systems::g_RenderSystem->ToggleDirectionalLight(lightEnabled);
        }
    }
}

void ImGuiManager::HelpMarker(const char* desc)
{
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

void ImGuiManager::RenderStats()
{
    // OpenGL statistics
    ImGui::Text("OpenGL Version: %s", glGetString(GL_VERSION));
    ImGui::Text("Vendor: %s", glGetString(GL_VENDOR));
    ImGui::Text("Renderer: %s", glGetString(GL_RENDERER));
    ImGui::Text("GLSL Version: %s", glGetString(GL_SHADING_LANGUAGE_VERSION));
    
    ImGui::Separator();
    
    ImGui::Separator();
    
    // Window info
    ImGui::Text("Window Size: %dx%d", m_Window.GetWidth(), m_Window.GetHeight());
} 