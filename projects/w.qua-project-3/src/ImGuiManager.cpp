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
#include "Bvh.hpp"

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
    
    // (Frustum visualization UI removed)

    ImGui::End();

    // ───────────────────────────────── Assignment 3 ───────────────────────────
    ImGui::Begin("Assignment 3");

    ImGui::Text("BVH Controls:");
    RenderBVHControls(registry);

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

    // Gun
    float gunScale = GetModelScale(registry, ModelType::Gun);
    if (ImGui::SliderFloat("Gun Scale", &gunScale, 0.1f, 5.0f, "%.2f"))
    {
        SetModelScale(registry, ModelType::Gun, gunScale);
    }

    // Cat
    float catScale = GetModelScale(registry, ModelType::Cat);
    if (ImGui::SliderFloat("Cat Scale", &catScale, 0.1f, 5.0f, "%.2f"))
    {
        SetModelScale(registry, ModelType::Cat, catScale);
    }

    // Arm
    float armScale = GetModelScale(registry, ModelType::Arm);
    if (ImGui::SliderFloat("Arm Scale", &armScale, 0.1f, 5.0f, "%.2f"))
    {
        SetModelScale(registry, ModelType::Arm, armScale);
    }

    // Stuffed
    float stuffedScale = GetModelScale(registry, ModelType::Stuffed);
    if (ImGui::SliderFloat("Stuffed Scale", &stuffedScale, 0.1f, 5.0f, "%.2f"))
    {
        SetModelScale(registry, ModelType::Stuffed, stuffedScale);
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


void ImGuiManager::RenderBVHControls(Registry& registry)
{
    if (!Systems::g_RenderSystem)
    {
        ImGui::Text("Render system not available");
        return;
    }

    // Build method selection 
    static int buildMethod = 0; // 0 = Top-down, 1 = Bottom-up
    static int prevBuildMethod = buildMethod;
    static int splitStrategy = 0, prevSplitStrategy = 0;
    static int termination   = 0, prevTermination   = 0;
    static int heuristic     = 0, prevHeuristic     = 0;

    ImGui::RadioButton("Top-Down", &buildMethod, 0); ImGui::SameLine();
    ImGui::RadioButton("Bottom-Up", &buildMethod, 1);

    ImGui::Separator();

    if (buildMethod == 0)
    {
        // Top-down parameters
        ImGui::Text("Top-Down Split Strategy:");
        ImGui::RadioButton("Median Centre", &splitStrategy, 0); ImGui::SameLine();
        ImGui::RadioButton("Median Extent", &splitStrategy, 1); ImGui::SameLine();
        ImGui::RadioButton("K-even", &splitStrategy, 2);

        ImGui::Text("Termination:");
        ImGui::RadioButton("Single Obj", &termination, 0); ImGui::SameLine();
        ImGui::RadioButton("Max Two", &termination, 1); ImGui::SameLine();
        ImGui::RadioButton("Height=2", &termination, 2);

    }
    else
    {
        // Bottom-up parameters
        ImGui::Text("Bottom-Up Heuristic:");
        ImGui::RadioButton("Nearest", &heuristic, 0); ImGui::SameLine();
        ImGui::RadioButton("Min Volume", &heuristic, 1); ImGui::SameLine();
        ImGui::RadioButton("Min SA", &heuristic, 2);

    }

    // Track visual mode previous value
    static int prevVisualMode = BvhBuildConfig::s_BuildWithAabb ? 0 : 1;

    // Detect changes in builder parameters (method/split/termination/heuristic)
    bool paramsChanged = (buildMethod != prevBuildMethod) ||
                         (splitStrategy != prevSplitStrategy) ||
                         (termination   != prevTermination)   ||
                         (heuristic     != prevHeuristic);

    static int visualMode = BvhBuildConfig::s_BuildWithAabb ? 0 : 1; // 0=AABB,1=Sphere
    ImGui::Separator();
    ImGui::Text("Build BVH With:");
    ImGui::RadioButton("AABB",   &visualMode, 0); ImGui::SameLine();
    ImGui::RadioButton("Bounding Sphere", &visualMode, 1);

    bool visualChanged = false;
    if (visualMode != prevVisualMode)
    {
        BvhBuildConfig::s_BuildWithAabb = (visualMode == 0);
        visualChanged = true;
        prevVisualMode = visualMode;
    }

    bool rebuildNeeded = paramsChanged || visualChanged;

    if (rebuildNeeded)
    {
        BvhBuildConfig::s_Method        = (buildMethod == 0) ? BvhBuildMethod::TopDown : BvhBuildMethod::BottomUp;
        BvhBuildConfig::s_TDStrategy    = static_cast<TDSSplitStrategy>(splitStrategy);
        BvhBuildConfig::s_TDTermination = static_cast<TDSTermination>(termination);
        BvhBuildConfig::s_BUHeuristic   = static_cast<BUSHeuristic>(heuristic);

        if (Systems::g_RenderSystem)
        {
            if (BvhBuildConfig::s_Method == BvhBuildMethod::TopDown)
            {
                Systems::g_RenderSystem->BuildBVH(BvhBuildConfig::s_Method,
                                                  BvhBuildConfig::s_TDStrategy,
                                                  BvhBuildConfig::s_TDTermination,
                                                  BvhBuildConfig::s_BUHeuristic,
                                                  BvhBuildConfig::s_BuildWithAabb);
            }
            else
            {
                Systems::g_RenderSystem->BuildBVH(BvhBuildConfig::s_Method,
                                                  BvhBuildConfig::s_TDStrategy,
                                                  BvhBuildConfig::s_TDTermination,
                                                  BvhBuildConfig::s_BUHeuristic,
                                                  BvhBuildConfig::s_BuildWithAabb);
            }
        }

        // Update previous trackers
        prevBuildMethod   = buildMethod;
        prevSplitStrategy = splitStrategy;
        prevTermination   = termination;
        prevHeuristic     = heuristic;
    }

    // Per-level visibility
    ImGui::Separator();
    ImGui::Text("Show Levels:");
    for (int lvl = 0; lvl < RenderSystem::kMaxBVHLevels; ++lvl)
    {
        bool lvlVis = Systems::g_RenderSystem->IsBVHLevelVisible(lvl);
        std::string label = "L" + std::to_string(lvl);
        if (ImGui::Checkbox(label.c_str(), &lvlVis))
        {
            Systems::g_RenderSystem->SetBVHLevelVisible(lvl, lvlVis);
        }
        if (lvl != RenderSystem::kMaxBVHLevels - 1) ImGui::SameLine();
    }

    ImGui::Separator();
} 