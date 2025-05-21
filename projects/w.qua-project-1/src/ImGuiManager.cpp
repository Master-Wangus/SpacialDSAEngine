#include "pch.h"
#include "ImGuiManager.hpp"
#include "Registry.hpp"
#include "Components.hpp"
#include "Primitives.hpp"
#include "CollisionSystem.hpp"
#include "Systems.hpp"

#include <GL/glew.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>
#include <iostream>

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
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    
    // Note: ImGuiConfigFlags_DockingEnable might not be available in older ImGui versions
    // Comment out if it's causing issues
    // io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;      // Enable Docking
    
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
    if (ImGui::CollapsingHeader("Collision Information", ImGuiTreeNodeFlags_DefaultOpen)) {
        RenderCollisionInfo(registry);
    }
    
    if (ImGui::CollapsingHeader("Entity List")) {
        RenderEntityList(registry);
    }
    
    if (ImGui::CollapsingHeader("Camera Settings")) {
        RenderCameraSettings(registry);
    }
    
    if (ImGui::CollapsingHeader("Debug Statistics")) {
        RenderStats();
    }
    
    ImGui::End();
}

void ImGuiManager::RenderCollisionInfo(Registry& registry)
{
    if (Systems::g_CollisionSystem) {
        const auto& collisions = Systems::g_CollisionSystem->GetCollisions();
        
        ImGui::Text("Active Collisions: %zu", collisions.size());
        ImGui::Separator();
        
        ImGui::BeginChild("CollisionsList", ImVec2(0, 150), true);
        
        for (size_t i = 0; i < collisions.size(); i++) {
            const auto& collision = collisions[i];
            ImGui::Text("Collision %zu: Entity %u <-> Entity %u", 
                i, static_cast<uint32_t>(collision.entity1), static_cast<uint32_t>(collision.entity2));
            
            if (ImGui::IsItemHovered()) {
                ImGui::BeginTooltip();
                if (registry.HasComponent<AABBComponent>(collision.entity1)) {
                    auto& aabb1 = registry.GetComponent<AABBComponent>(collision.entity1);
                    ImGui::Text("Entity %u AABB: Center (%.2f, %.2f, %.2f), Half-Extents (%.2f, %.2f, %.2f)",
                        static_cast<uint32_t>(collision.entity1),
                        aabb1.m_AABB.m_Center.x, aabb1.m_AABB.m_Center.y, aabb1.m_AABB.m_Center.z,
                        aabb1.m_AABB.m_HalfExtents.x, aabb1.m_AABB.m_HalfExtents.y, aabb1.m_AABB.m_HalfExtents.z);
                }
                if (registry.HasComponent<AABBComponent>(collision.entity2)) {
                    auto& aabb2 = registry.GetComponent<AABBComponent>(collision.entity2);
                    ImGui::Text("Entity %u AABB: Center (%.2f, %.2f, %.2f), Half-Extents (%.2f, %.2f, %.2f)",
                        static_cast<uint32_t>(collision.entity2),
                        aabb2.m_AABB.m_Center.x, aabb2.m_AABB.m_Center.y, aabb2.m_AABB.m_Center.z,
                        aabb2.m_AABB.m_HalfExtents.x, aabb2.m_AABB.m_HalfExtents.y, aabb2.m_AABB.m_HalfExtents.z);
                }
                ImGui::EndTooltip();
            }
        }
        
        ImGui::EndChild();
    }
}

void ImGuiManager::RenderEntityList(Registry& registry)
{
    auto transformView = registry.View<TransformComponent>();
    
    ImGui::Text("Entities: %zu", transformView.size());
    ImGui::Separator();
    
    ImGui::BeginChild("EntitiesList", ImVec2(0, 150), true);
    
    for (auto entity : transformView) {
        auto& transform = registry.GetComponent<TransformComponent>(entity);
        bool hasAABB = registry.HasComponent<AABBComponent>(entity);
        bool hasSphere = registry.HasComponent<BoundingSphereComponent>(entity);
        
        ImGui::PushID(static_cast<int>(entity));
        
        std::string entityName = "Entity " + std::to_string(static_cast<uint32_t>(entity));
        if (ImGui::TreeNode(entityName.c_str())) {
            ImGui::Text("Position: (%.2f, %.2f, %.2f)", 
                transform.m_Position.x, transform.m_Position.y, transform.m_Position.z);
            ImGui::Text("Rotation: (%.2f, %.2f, %.2f)", 
                transform.m_Rotation.x, transform.m_Rotation.y, transform.m_Rotation.z);
            ImGui::Text("Scale: (%.2f, %.2f, %.2f)", 
                transform.m_Scale.x, transform.m_Scale.y, transform.m_Scale.z);
                
            ImGui::Separator();
            
            // Edit transform
            float pos[3] = { transform.m_Position.x, transform.m_Position.y, transform.m_Position.z };
            if (ImGui::DragFloat3("Position", pos, 0.1f)) {
                transform.m_Position = glm::vec3(pos[0], pos[1], pos[2]);
                transform.UpdateModelMatrix();
            }
            
            ImGui::Text("Colliders: %s%s", 
                hasAABB ? "AABB " : "", 
                hasSphere ? "Sphere" : "");
                
            ImGui::TreePop();
        }
        
        ImGui::PopID();
    }
    
    ImGui::EndChild();
}

void ImGuiManager::RenderCameraSettings(Registry& registry)
{
    auto cameraView = registry.View<CameraComponent>();
    
    if (!cameraView.empty()) {
        auto cameraEntity = *cameraView.begin();
        auto& camera = registry.GetComponent<CameraComponent>(cameraEntity);
        
        // Display camera info
        ImGui::Text("Camera Position: (%.2f, %.2f, %.2f)", 
            camera.m_FPS.m_CameraPosition.x, 
            camera.m_FPS.m_CameraPosition.y, 
            camera.m_FPS.m_CameraPosition.z);
            
        ImGui::Text("Camera Front: (%.2f, %.2f, %.2f)", 
            camera.m_FPS.m_CameraFront.x, 
            camera.m_FPS.m_CameraFront.y, 
            camera.m_FPS.m_CameraFront.z);
            
        ImGui::Text("Yaw: %.2f, Pitch: %.2f", 
            camera.m_FPS.m_YawAngle, 
            camera.m_FPS.m_PitchAngle);
            
        ImGui::Separator();
        
        // Edit camera settings
        float speed = camera.m_FPS.m_MovementSpeed;
        if (ImGui::SliderFloat("Movement Speed", &speed, 1.0f, 10.0f)) {
            camera.m_FPS.m_MovementSpeed = speed;
        }
        
        float sensitivity = camera.m_FPS.m_MouseSensitivity;
        if (ImGui::SliderFloat("Mouse Sensitivity", &sensitivity, 0.05f, 1.0f)) {
            camera.m_FPS.m_MouseSensitivity = sensitivity;
        }
        
        float fov = camera.m_Projection.m_Fov;
        if (ImGui::SliderFloat("Field of View", &fov, 30.0f, 120.0f)) {
            camera.m_Projection.m_Fov = fov;
        }
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