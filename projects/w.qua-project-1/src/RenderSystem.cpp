#include "RenderSystem.hpp"
#include "Shader.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

RenderSystem::RenderSystem(Registry& registry, Window& window, const std::shared_ptr<Shader>& shader)
    : m_Registry(registry), m_Window(window), m_Shader(shader)
{
    // Set up framebuffer resize callback
    window.SetFramebufferSizeCallback([](int width, int height) {
        // Update viewport when window is resized
        glViewport(0, 0, width, height);
    });
}

void RenderSystem::Initialize()
{
    // Set initial viewport size
    glViewport(0, 0, m_Window.GetWidth(), m_Window.GetHeight());
    
    // Initialize all renderable objects
    for (auto entity : m_Registry.View<RenderComponent>()) 
    {
        m_Registry.GetComponent<RenderComponent>(entity).m_Renderable->Initialize(m_Shader);
    }
    SetupLighting();
}

void RenderSystem::Render()
{
    // Find the camera to use for rendering
    auto cameraView = m_Registry.View<CameraComponent>();
    if (cameraView.empty()) return;
    
    auto cameraEntity = *cameraView.begin();
    auto& camera = m_Registry.GetComponent<CameraComponent>(cameraEntity);
    
    float aspectRatio = static_cast<float>(m_Window.GetWidth()) / static_cast<float>(m_Window.GetHeight());
    glm::mat4 viewMatrix = camera.GetViewMatrix();
    glm::mat4 projectionMatrix = camera.GetProjectionMatrix(aspectRatio);
    
    glm::vec3 cameraPosition = camera.GetPosition();
    
    auto renderView = m_Registry.View<TransformComponent, RenderComponent>();
    for (auto entity : renderView)
    {
        auto& transform = m_Registry.GetComponent<TransformComponent>(entity);
        auto& renderComp = m_Registry.GetComponent<RenderComponent>(entity);
        
        renderComp.m_Renderable->Render(transform.m_Model, viewMatrix, projectionMatrix);
    }
}

void RenderSystem::Shutdown()
{
    for (auto entity : m_Registry.View<RenderComponent>()) 
    {
        m_Registry.GetComponent<RenderComponent>(entity).m_Renderable->CleanUp();
    }
}

void RenderSystem::SetupLighting()
{
    auto lightView = m_Registry.View<DirectionalLightComponent>();
    DirectionalLight light;
    
    if (lightView.empty())
    {
        // Create new light
        auto lightEntity = m_Registry.Create();
        
        // Direction (world space) - with a clear downward component
        light.m_Direction = glm::vec4(-0.2f, -1.0f, -0.3f, 0.0f);
        
        // Light color - bright white
        light.m_Color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
        
        // Add the light component to the entity
        m_Registry.AddComponent<DirectionalLightComponent>(lightEntity, DirectionalLightComponent(light));
    }
    else
    {
        // Use existing light
        light = m_Registry.GetComponent<DirectionalLightComponent>(*lightView.begin()).m_Light;
    }
    
    // Set up UBO for the light using Buffer class methods
    static GLuint lightUBO = 0;
    if (lightUBO == 0) 
    {
        lightUBO = Buffer::CreateUniformBuffer(sizeof(DirectionalLight), 0);
        Buffer::UpdateUniformBuffer(lightUBO, &light, sizeof(DirectionalLight));
    } 
    else 
    {
        // Update the data
        Buffer::UpdateUniformBuffer(lightUBO, &light, sizeof(DirectionalLight));
    }
} 