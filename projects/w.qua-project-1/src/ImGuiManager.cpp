#include "pch.h"
#include "ImGuiManager.hpp"
#include "Registry.hpp"
#include "Components.hpp"
#include "Primitives.hpp"
#include "CollisionSystem.hpp"
#include "Systems.hpp"
#include "Shader.hpp"
#include "RenderSystem.hpp"
#include "Buffer.hpp"

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
    float currentTime = (float)glfwGetTime();
    float deltaTime = currentTime - m_LastFrameTime;
    m_LastFrameTime = currentTime;
    
    m_FrameCount++;
    m_FrameTimeAccumulator += deltaTime;
    
    if (m_FrameTimeAccumulator >= 1.0f) {
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
    // Get the current selected scene
    static const char* currentScene = "Sphere vs Sphere";
    static int selectedScene = static_cast<int>(Systems::g_CurrentDemoScene);
    
    // Map the enum to descriptive strings
    static const char* sceneNames[] = {
        "Sphere vs Sphere",
        "AABB vs Sphere",
        "Sphere vs AABB",
        "AABB vs AABB",
        "Point vs Sphere",
        "Point vs AABB",
        "Point vs Triangle",
        "Point vs Plane",
        "Ray vs Plane",
        "Ray vs AABB",
        "Ray vs Sphere",
        "Ray vs Triangle",
        "Plane vs AABB",
        "Plane vs Sphere"
    };
    
    ImGui::Text("Current Scene: %s", sceneNames[selectedScene]);
    ImGui::Separator();
    
    if (ImGui::TreeNode("Volume-Based Tests"))
    {
        if (ImGui::Selectable("Sphere vs Sphere", selectedScene == static_cast<int>(DemoSceneType::SphereVsSphere)))
        {
            selectedScene = static_cast<int>(DemoSceneType::SphereVsSphere);
            SwitchScene(registry, static_cast<DemoSceneType>(selectedScene));
        }
        
        if (ImGui::Selectable("AABB vs Sphere", selectedScene == static_cast<int>(DemoSceneType::AABBVsSphere)))
        {
            selectedScene = static_cast<int>(DemoSceneType::AABBVsSphere);
            SwitchScene(registry, static_cast<DemoSceneType>(selectedScene));
        }
        
        if (ImGui::Selectable("Sphere vs AABB", selectedScene == static_cast<int>(DemoSceneType::SphereVsAABB)))
        {
            selectedScene = static_cast<int>(DemoSceneType::SphereVsAABB);
            SwitchScene(registry, static_cast<DemoSceneType>(selectedScene));
        }
        
        if (ImGui::Selectable("AABB vs AABB", selectedScene == static_cast<int>(DemoSceneType::AABBVsAABB)))
        {
            selectedScene = static_cast<int>(DemoSceneType::AABBVsAABB);
            SwitchScene(registry, static_cast<DemoSceneType>(selectedScene));
        }
        
        ImGui::TreePop();
    }
    
    if (ImGui::TreeNode("Point-Based Tests"))
    {
        if (ImGui::Selectable("Point vs Sphere", selectedScene == static_cast<int>(DemoSceneType::PointVsSphere)))
        {
            selectedScene = static_cast<int>(DemoSceneType::PointVsSphere);
            SwitchScene(registry, static_cast<DemoSceneType>(selectedScene));
        }
        
        if (ImGui::Selectable("Point vs AABB", selectedScene == static_cast<int>(DemoSceneType::PointVsAABB)))
        {
            selectedScene = static_cast<int>(DemoSceneType::PointVsAABB);
            SwitchScene(registry, static_cast<DemoSceneType>(selectedScene));
        }
        
        if (ImGui::Selectable("Point vs Triangle", selectedScene == static_cast<int>(DemoSceneType::PointVsTriangle)))
        {
            selectedScene = static_cast<int>(DemoSceneType::PointVsTriangle);
            SwitchScene(registry, static_cast<DemoSceneType>(selectedScene));
        }
        
        if (ImGui::Selectable("Point vs Plane", selectedScene == static_cast<int>(DemoSceneType::PointVsPlane)))
        {
            selectedScene = static_cast<int>(DemoSceneType::PointVsPlane);
            SwitchScene(registry, static_cast<DemoSceneType>(selectedScene));
        }
        
        ImGui::TreePop();
    }
    
    if (ImGui::TreeNode("Ray-Based Tests"))
    {
        if (ImGui::Selectable("Ray vs Plane", selectedScene == static_cast<int>(DemoSceneType::RayVsPlane)))
        {
            selectedScene = static_cast<int>(DemoSceneType::RayVsPlane);
            SwitchScene(registry, static_cast<DemoSceneType>(selectedScene));
        }
        
        if (ImGui::Selectable("Ray vs AABB", selectedScene == static_cast<int>(DemoSceneType::RayVsAABB)))
        {
            selectedScene = static_cast<int>(DemoSceneType::RayVsAABB);
            SwitchScene(registry, static_cast<DemoSceneType>(selectedScene));
        }
        
        if (ImGui::Selectable("Ray vs Sphere", selectedScene == static_cast<int>(DemoSceneType::RayVsSphere)))
        {
            selectedScene = static_cast<int>(DemoSceneType::RayVsSphere);
            SwitchScene(registry, static_cast<DemoSceneType>(selectedScene));
        }
        
        if (ImGui::Selectable("Ray vs Triangle", selectedScene == static_cast<int>(DemoSceneType::RayVsTriangle)))
        {
            selectedScene = static_cast<int>(DemoSceneType::RayVsTriangle);
            SwitchScene(registry, static_cast<DemoSceneType>(selectedScene));
        }
        
        ImGui::TreePop();
    }
    
    if (ImGui::TreeNode("Plane-Based Tests"))
    {
        if (ImGui::Selectable("Plane vs AABB", selectedScene == static_cast<int>(DemoSceneType::PlaneVsAABB)))
        {
            selectedScene = static_cast<int>(DemoSceneType::PlaneVsAABB);
            SwitchScene(registry, static_cast<DemoSceneType>(selectedScene));
        }
        
        if (ImGui::Selectable("Plane vs Sphere", selectedScene == static_cast<int>(DemoSceneType::PlaneVsSphere)))
        {
            selectedScene = static_cast<int>(DemoSceneType::PlaneVsSphere);
            SwitchScene(registry, static_cast<DemoSceneType>(selectedScene));
        }
        
        ImGui::TreePop();
    }
}

void ImGuiManager::SwitchScene(Registry& registry, DemoSceneType sceneType)
{
    // Get the shader from the render system (assuming it's already initialized)
    auto& renderSystem = Systems::g_RenderSystem;
    if (!renderSystem) return;
    
    auto shader = renderSystem->GetShader();
    if (!shader) return;
    
    // Switch to the selected scene
    Systems::SwitchScene(registry, m_Window, shader, sceneType);
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
        
        ImGui::SameLine();
        HelpMarker("Toggle directional lighting to see objects with or without shading");
        
        // Add ambient lighting controls
        ImGui::Separator();
        ImGui::Text("Ambient Lighting Controls");
        
        // Ambient intensity slider
        if (ImGui::SliderFloat("Ambient Intensity", &ambientIntensity, 0.0f, 1.0f))
        {
            // Update the material UBO with just the ambient intensity
            Buffer::UpdateUniformBuffer(materialUBO, &ambientIntensity, 
                                        sizeof(float), offsetof(Material, m_AmbientIntensity));
        }
        
        // Ambient color control
        if (ImGui::ColorEdit3("Ambient Color", ambientColor))
        {
            // Update the material UBO with just the ambient color
            glm::vec3 color(ambientColor[0], ambientColor[1], ambientColor[2]);
            Buffer::UpdateUniformBuffer(materialUBO, &color, 
                                        sizeof(glm::vec3), offsetof(Material, m_AmbientColor));
        }
        
        if (!lightEnabled)
        {
            ImGui::TextWrapped("Directional lighting is disabled. Objects are illuminated using ambient lighting only.");
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
    
    // Memory usage (this is a placeholder - implement actual memory tracking if needed)
    ImGui::Text("Memory Usage: N/A");
    
    ImGui::Separator();
    
    // Window info
    ImGui::Text("Window Size: %dx%d", m_Window.GetWidth(), m_Window.GetHeight());
} 