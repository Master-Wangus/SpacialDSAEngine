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
#include "Keybinds.hpp"
// (Removed BVH dependency)

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
    ImGui_ImplGlfw_InitForOpenGL((GLFWwindow*)m_Window.GetNativeWindow(), false);
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
    
    // Create a separate widget for model controls
    ImGui::Begin("Model Controls");
    

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
    
    ImGui::End();

    // (BVH UI removed)
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
    
    const char* currentCameraType = (camera.m_ActiveCameraType == CameraType::FPS) ? "FPS Camera" : "Top-Down Camera";
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
        ImGui::Text("C - Switch to Top-Down camera");
        
        ImGui::Separator();
        
        // FPS Camera details
        ImGui::Text("Yaw: %.1f°", camera.m_FPS.m_YawAngle);
        ImGui::Text("Pitch: %.1f°", camera.m_FPS.m_PitchAngle);
        ImGui::Text("Speed: %.2f", camera.m_FPS.m_MovementSpeed);
    }
    else if (camera.m_ActiveCameraType == CameraType::Orbital) 
    {
        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.3f, 1.0f), "Top-Down Camera Controls:");
        ImGui::Text("Mouse + Right Click - Pan view");
        ImGui::Text("Mouse Wheel - Zoom in/out");
        ImGui::Text("WASD - Move target point");
        ImGui::Text("C - Switch to FPS camera");
        
        ImGui::Separator();
        
        // Top-Down Camera details
        glm::vec3 target = camera.m_TopDown.m_Target;
        ImGui::Text("Target: (%.2f, %.2f, %.2f)", target.x, target.y, target.z);
        ImGui::Text("Height: %.2f", camera.m_TopDown.m_Distance);
        ImGui::Text("Zoom Range: %.1f - %.1f", camera.m_TopDown.m_MinDistance, camera.m_TopDown.m_MaxDistance);
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

        // Slider to control light rotation speed (degrees per second)
        float speedDeg = glm::degrees(Systems::g_RenderSystem->GetLightRotationSpeed());
        if (ImGui::SliderFloat("Light Speed (deg/s)", &speedDeg, 0.0f, 180.0f))
        {
            Systems::g_RenderSystem->SetLightRotationSpeed(glm::radians(speedDeg));
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
    
    // Bounding Sphere (PCA) Controls
    bool showPCA = Systems::g_RenderSystem->IsPCASphereVisible();
    if (ImGui::Checkbox("Bounding Sphere", &showPCA))
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
    ImGui::Text("UNC Section Scales:");

    float section4Scale = DemoScene::GetSectionScale(registry, SectionId::Section4);
    if (ImGui::SliderFloat("Section 4 Scale", &section4Scale, 0.05f, 5.0f, "%.2f"))
    {
        DemoScene::SetSectionScale(registry, SectionId::Section4, section4Scale);
    }

    float section5Scale = DemoScene::GetSectionScale(registry, SectionId::Section5);
    if (ImGui::SliderFloat("Section 5 Scale", &section5Scale, 0.05f, 5.0f, "%.2f"))
    {
        DemoScene::SetSectionScale(registry, SectionId::Section5, section5Scale);
    }

    float section6Scale = DemoScene::GetSectionScale(registry, SectionId::Section6);
    if (ImGui::SliderFloat("Section 6 Scale", &section6Scale, 0.05f, 5.0f, "%.2f"))
    {
        DemoScene::SetSectionScale(registry, SectionId::Section6, section6Scale);
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


// (Removed RenderBVHControls implementation) 