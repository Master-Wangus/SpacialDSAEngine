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
    
    // Calculate delta time for FPS counter
    float currentTime = static_cast<float>(m_Window.GetTime());
    float deltaTime = currentTime - m_LastFrameTime;
    m_LastFrameTime = currentTime;
    
    // Display FPS
    UpdateFrameRate(deltaTime);
    ImGui::Text("FPS: %.1f", m_FrameRate);
    
    ImGui::Separator();
    
    if (ImGui::Button("Reset Scene")) {
        Systems::ResetCurrentScene(registry, m_Window);
    }
    
    ImGui::Separator();
    
    // Add collapsing headers for different control categories
    if (ImGui::CollapsingHeader("Camera Controls", ImGuiTreeNodeFlags_DefaultOpen)) {
        RenderCameraControls(registry);
    }
    
    if (ImGui::CollapsingHeader("Lighting Controls")) {
        RenderLightingControls(registry);
    }
    
    if (ImGui::CollapsingHeader("System Information")) {
        RenderStats();
    }
    
    ImGui::End();
    
    // Create a separate widget for Assignment 2 controls
    ImGui::Begin("Assignment 2");
    
    // Model selection controls removed; all models are shown simultaneously.

    // (Removed UI controls for switching models)

    ImGui::Separator();
    
    // Bounding volume controls
    ImGui::Text("Bounding Volume Controls:");
    RenderBoundingVolumeControls(registry);
    
    ImGui::Separator();
    
    // Wireframe controls
    ImGui::Text("Rendering Controls:");
    RenderWireframeControls(registry);
    
    ImGui::Separator();
    
    // Object visibility
    ImGui::Text("Object Visibility:");
    RenderObjectVisibilityControls(registry);

    ImGui::Separator();
    
    // Scaling controls
    ImGui::Text("Scale Models (Uniform):");
    RenderScalingControls(registry);

    ImGui::Separator();
    
    // Frustum visualization controls
    ImGui::Text("Frustum Visualization:");
    RenderFrustumControls(registry);
    
    // Add color legend for frustum culling
    ImGui::Separator();
    ImGui::Text("Frustum Culling Color Legend:");
    
    // Inside color (Green)
    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Green - Inside Frustum");
    
    // Outside color (Red)
    ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Red - Outside Frustum");
    
    // Intersecting color (Yellow)
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Yellow - Intersecting Frustum");
    
    ImGui::End();
}

void ImGuiManager::RenderCameraControls(Registry& registry)
{
    auto cameraView = registry.View<CameraComponent>();
    if (cameraView.empty()) 
    {
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
    if (Systems::g_RenderSystem)
    {
        bool lightEnabled = Systems::g_RenderSystem->IsDirectionalLightEnabled();
        
        if (ImGui::Checkbox("Enable Directional Lighting", &lightEnabled))
        {
            Systems::g_RenderSystem->ToggleDirectionalLight(lightEnabled);
        }
    }
}

void ImGuiManager::RenderBoundingVolumeControls(Registry& registry)
{
    if (!Systems::g_RenderSystem) 
    {
        ImGui::Text("Render system not available");
        return;
    }
    
    ImGui::Text("Toggle Bounding Volume Visibility:");

    // AABB Controls
    bool showAABB = Systems::g_RenderSystem->IsAABBVisible();
    if (ImGui::Checkbox("AABB", &showAABB))
    {
        Systems::g_RenderSystem->SetShowAABB(showAABB);
    }
    
    // Ritter Sphere Controls
    bool showRitter = Systems::g_RenderSystem->IsRitterSphereVisible();
    if (ImGui::Checkbox("Ritter Sphere", &showRitter))
    {
        Systems::g_RenderSystem->SetShowRitterSphere(showRitter);
    }
    
    // Larsson Sphere Controls
    bool showLarsson = Systems::g_RenderSystem->IsLarsonSphereVisible();
    if (ImGui::Checkbox("Larsson Iterative Sphere", &showLarsson))
    {
        Systems::g_RenderSystem->SetShowLarsonSphere(showLarsson);
    }
    
    // PCA Sphere Controls
    bool showPCA = Systems::g_RenderSystem->IsPCASphereVisible();
    if (ImGui::Checkbox("PCA Sphere", &showPCA))
    {
        Systems::g_RenderSystem->SetShowPCASphere(showPCA);
    }
    
    // OBB Controls
    bool showOBB = Systems::g_RenderSystem->IsOBBVisible();
    if (ImGui::Checkbox("PCA OBB", &showOBB))
    {
        Systems::g_RenderSystem->SetShowOBB(showOBB);
    }
    
    ImGui::Separator();
}

void ImGuiManager::RenderWireframeControls(Registry& registry)
{
    if (!Systems::g_RenderSystem) 
    {
        ImGui::Text("Render system not available");
        return;
    }
    
    bool wireframeEnabled = Systems::g_RenderSystem->IsGlobalWireframeEnabled();
    if (ImGui::Checkbox("Global Wireframe Mode", &wireframeEnabled))
    {
        Systems::g_RenderSystem->SetGlobalWireframe(wireframeEnabled);
    }
}

void ImGuiManager::RenderObjectVisibilityControls(Registry& registry)
{    
    bool showMainObjects = Systems::g_RenderSystem->IsShowMainObjects();
    
    // Object visibility toggle
    if (ImGui::Checkbox("Show Main Objects", &showMainObjects))
    {
        Systems::g_RenderSystem->SetShowMainObjects(showMainObjects);
    }
}

void ImGuiManager::RenderScalingControls(Registry& registry)
{
    using namespace DemoScene;

    // Rhino
    float rhinoScale = GetModelScale(registry, ModelType::Rhino);
    if (ImGui::SliderFloat("Rhino Scale", &rhinoScale, 0.1f, 5.0f, "%.2f"))
    {
        SetModelScale(registry, ModelType::Rhino, rhinoScale);
    }

    // Cup
    float cupScale = GetModelScale(registry, ModelType::Cup);
    if (ImGui::SliderFloat("Cup Scale", &cupScale, 0.1f, 5.0f, "%.2f"))
    {
        SetModelScale(registry, ModelType::Cup, cupScale);
    }

    // Bunny
    float bunnyScale = GetModelScale(registry, ModelType::Bunny);
    if (ImGui::SliderFloat("Bunny Scale", &bunnyScale, 0.1f, 5.0f, "%.2f"))
    {
        SetModelScale(registry, ModelType::Bunny, bunnyScale);
    }

    // Cube
    float cubeScale = GetModelScale(registry, ModelType::Cube);
    if (ImGui::SliderFloat("Cube Scale", &cubeScale, 0.1f, 5.0f, "%.2f"))
    {
        SetModelScale(registry, ModelType::Cube, cubeScale);
    }
}

void ImGuiManager::RenderFrustumControls(Registry& registry)
{
    if (!Systems::g_RenderSystem) 
    {
        ImGui::Text("Render system not available");
        return;
    }
    
    // Frustum visualization toggle
    bool showFrustum = Systems::g_RenderSystem->IsShowFrustum();
    if (ImGui::Checkbox("Show View Frustum", &showFrustum))
    {
        Systems::g_RenderSystem->SetShowFrustum(showFrustum);
    }
    
    // Get reference camera projection for frustum visualization (separate from main camera)
    if (Systems::g_CameraSystem) 
    {
        Projection referenceProjection = Systems::g_CameraSystem->GetReferenceCameraProjection();
        
        ImGui::Separator();
        ImGui::Text("Reference Frustum Parameters:");
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "(These control the visualized frustum, not the main camera)");
        
        // Field of View control
        float fov = referenceProjection.m_Fov;
        if (ImGui::SliderFloat("Field of View (degrees)", &fov, 10.0f, 170.0f, "%.1f")) {
            referenceProjection.m_Fov = fov;
            Systems::g_CameraSystem->SetReferenceCameraProjection(referenceProjection);
        }
        
        // Display current values for reference
        ImGui::Separator();
        ImGui::Text("Reference Camera Values:");
        ImGui::Text("FOV: %.1f degrees", referenceProjection.m_Fov);
    }
    else 
    {
        ImGui::Text("Camera system not available");
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

void ImGuiManager::UpdateFrameRate(float deltaTime)
{
    m_FrameCount++;
    m_FrameTimeAccumulator += deltaTime;
    
    if (m_FrameTimeAccumulator >= 1.0f) 
    {
        m_FrameRate = static_cast<float>(m_FrameCount) / m_FrameTimeAccumulator;
        m_FrameCount = 0;
        m_FrameTimeAccumulator = 0.0f;
    }
}

// Stub: model selection UI removed
void ImGuiManager::RenderModelSelectionControls(Registry& /*registry*/)
{
    // No model-selection UI; all models are visible by default.
} 