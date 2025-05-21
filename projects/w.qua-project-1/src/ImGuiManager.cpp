#include "pch.h"
#include "ImGuiManager.hpp"
#include "Registry.hpp"
#include "Components.hpp"
#include "Primitives.hpp"
#include "CollisionSystem.hpp"
#include "Systems.hpp"

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
    
    // Main sections
    
    if (ImGui::CollapsingHeader("Debug Statistics")) {
        RenderStats();
    }
    
    ImGui::End();
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