/**
 * @class RenderSystem
 * @brief System for rendering 3D objects and scenes.
 *
 * This system manages the rendering process of 3D objects in the scene,
 * including camera view, projection setup, and proper draw call ordering.
 */

#include "RenderSystem.hpp"
#include "Shader.hpp"
#include "SphereRenderer.hpp"
#include "Components.hpp"
#include "Registry.hpp"
#include "Window.hpp"
#include "Buffer.hpp"
#include "Lighting.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

RenderSystem::RenderSystem(Registry& registry, Window& window, const std::shared_ptr<Shader>& shader)
    : m_Registry(registry), m_Window(window), m_Shader(shader)
{
    // Set up framebuffer resize callback
    window.SetFramebufferSizeCallback([](int width, int height)
        {
        // Update viewport when window is resized
        glViewport(0, 0, width, height);
    });
}

void RenderSystem::Initialize()
{
    glViewport(0, 0, m_Window.GetWidth(), m_Window.GetHeight());
    
    for (auto entity : m_Registry.View<RenderComponent>()) 
    {
        m_Registry.GetComponent<RenderComponent>(entity).m_Renderable->Initialize(m_Shader);
    }
    SetupLighting();
    
    SetupMaterial();
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
    
    // Set the camera position uniform for lighting calculations
    m_Shader->Use();
    m_Shader->SetVec3("viewPos", cameraPosition);
    
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
        m_LightEntity = m_Registry.Create();
        light.m_Direction = glm::vec4(-0.2f, -1.0f, -0.3f, 0.0f);
        light.m_Color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
        light.m_Enabled = 1.0f;  
        m_Registry.AddComponent<DirectionalLightComponent>(m_LightEntity, DirectionalLightComponent(light));
    }
    else
    {
        m_LightEntity = *lightView.begin();
        light = m_Registry.GetComponent<DirectionalLightComponent>(m_LightEntity).m_Light;
    }
    
    // Create a visual representation of the light source
    CreateLightSourceVisualization(light);
    
    UpdateLighting();
}

void RenderSystem::CreateLightSourceVisualization(const DirectionalLight& light)
{

    // Calculate position for the light source visualization
    // Place it in the opposite direction of the light direction
    glm::vec3 lightDirection = glm::normalize(glm::vec3(light.m_Direction));
    glm::vec3 lightPosition = -lightDirection * 5.0f; // Place 5 units away
    
    // Create a small yellow sphere to represent the light source
    m_LightVisualizationEntity = m_Registry.Create();
    
    auto lightSphereRenderer = std::make_shared<SphereRenderer>(
        lightPosition, 
        0.2f,  // Small radius
        glm::vec3(1.0f, 1.0f, 0.0f)  // Yellow color
    );
    lightSphereRenderer->Initialize(m_Shader);
    
    m_Registry.AddComponent<TransformComponent>(m_LightVisualizationEntity, 
        TransformComponent(lightPosition, glm::vec3(0.0f), glm::vec3(0.2f)));
    m_Registry.AddComponent<RenderComponent>(m_LightVisualizationEntity, 
        RenderComponent(lightSphereRenderer));
    
    // Add collision component so the light can be dragged
    m_Registry.AddComponent<CollisionComponent>(m_LightVisualizationEntity, 
        CollisionComponent::CreateSphere(lightPosition, 0.2f));
    
}

void RenderSystem::SetupMaterial()
{
    Material material;
    
    material.m_AmbientColor = glm::vec3(1.0f, 1.0f, 1.0f);
    material.m_AmbientIntensity = 0.5f;
    
    material.m_DiffuseColor = glm::vec3(1.0f, 1.0f, 1.0f);
    material.m_DiffuseIntensity = 1.0f;
    material.m_SpecularColor = glm::vec3(0.5f, 0.5f, 0.5f);
    material.m_SpecularIntensity = 0.5f;
    material.m_Shininess = 32.0f;
    
    static GLuint materialUBO = 0;
    if (materialUBO == 0) 
    {
        materialUBO = Buffer::CreateUniformBuffer(sizeof(Material), 1);
        Buffer::UpdateUniformBuffer(materialUBO, &material, sizeof(Material));
    } 
    else 
    {
        Buffer::UpdateUniformBuffer(materialUBO, &material, sizeof(Material));
    }
}

void RenderSystem::UpdateLighting()
{
    if (m_LightEntity != entt::null && m_Registry.HasComponent<DirectionalLightComponent>(m_LightEntity))
    {
        DirectionalLight light = m_Registry.GetComponent<DirectionalLightComponent>(m_LightEntity).m_Light;
        
        static GLuint lightUBO = 0;
        if (lightUBO == 0) 
        {
            lightUBO = Buffer::CreateUniformBuffer(sizeof(DirectionalLight), 0);
            Buffer::UpdateUniformBuffer(lightUBO, &light, sizeof(DirectionalLight));
        } 
        else 
        {
            Buffer::UpdateUniformBuffer(lightUBO, &light, sizeof(DirectionalLight));
        }
    }
}

void RenderSystem::ToggleDirectionalLight(bool enabled)
{
    if (m_LightEntity != entt::null && m_Registry.HasComponent<DirectionalLightComponent>(m_LightEntity))
    {
        auto& lightComp = m_Registry.GetComponent<DirectionalLightComponent>(m_LightEntity);
        lightComp.m_Light.m_Enabled = enabled ? 1.0f : 0.0f;
        
        UpdateLighting();
    }
}

bool RenderSystem::IsDirectionalLightEnabled() const
{
    if (m_LightEntity != entt::null && m_Registry.HasComponent<DirectionalLightComponent>(m_LightEntity))
    {
        auto& lightComp = m_Registry.GetComponent<DirectionalLightComponent>(m_LightEntity);
        return lightComp.m_Light.m_Enabled > 0.5f;
    }
    
    return false;
}

void RenderSystem::UpdateLightFromVisualization()
{
    if (m_LightEntity != entt::null && 
        m_LightVisualizationEntity != entt::null &&
        m_Registry.HasComponent<DirectionalLightComponent>(m_LightEntity) &&
        m_Registry.HasComponent<TransformComponent>(m_LightVisualizationEntity))
    {
        auto& lightComp = m_Registry.GetComponent<DirectionalLightComponent>(m_LightEntity);
        auto& lightTransform = m_Registry.GetComponent<TransformComponent>(m_LightVisualizationEntity);
        
        glm::vec3 lightPosition = lightTransform.m_Position;
        glm::vec3 sceneCenter = glm::vec3(0.0f, 0.0f, 0.0f); 
        glm::vec3 lightDirection = glm::normalize(sceneCenter - lightPosition);
        
        lightComp.m_Light.m_Direction = glm::vec4(lightDirection, 0.0f);
        
        UpdateLighting();
    }
} 