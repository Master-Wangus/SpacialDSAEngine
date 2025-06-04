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
#include "CubeRenderer.hpp"
#include "Components.hpp"
#include "Shapes.hpp"
#include "Registry.hpp"
#include "Window.hpp"
#include "Buffer.hpp"
#include "Lighting.hpp"

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
        auto& renderComp = m_Registry.GetComponent<RenderComponent>(entity);
        
        if (renderComp.m_Renderable) 
        {
            renderComp.m_Renderable->Initialize(m_Shader);
        }
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
    
    // Set polygon mode for main objects to always be wireframe
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    
    // Render entities with both transform and render components
    auto renderView = m_Registry.View<TransformComponent, RenderComponent>();
    for (auto entity : renderView) 
    {
        auto& transform = m_Registry.GetComponent<TransformComponent>(entity);
        auto& renderComp = m_Registry.GetComponent<RenderComponent>(entity);
        
        // Only render main objects if they should be visible
        if (m_ShowMainObjects && renderComp.m_Renderable) 
        {
            renderComp.m_Renderable->Render(transform.m_Model, viewMatrix, projectionMatrix);
        }
        
        // Render bounding volumes if enabled and entity has BoundingComponent
        if (m_Registry.HasComponent<BoundingComponent>(entity))
        {
            // Reset to solid mode for bounding volume wireframes (they use line primitives)
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            
            auto& boundingComp = m_Registry.GetComponent<BoundingComponent>(entity);
            
            if (m_ShowAABB && boundingComp.m_AABBRenderable) {
                boundingComp.m_AABBRenderable->Render(transform.m_Model, viewMatrix, projectionMatrix);
            }
            
            if (m_ShowRitterSphere && boundingComp.m_RitterRenderable) {
                boundingComp.m_RitterRenderable->Render(transform.m_Model, viewMatrix, projectionMatrix);
            }
            
            if (m_ShowLarsonSphere && boundingComp.m_LarsonRenderable) {
                boundingComp.m_LarsonRenderable->Render(transform.m_Model, viewMatrix, projectionMatrix);
            }
            
            if (m_ShowPCASphere && boundingComp.m_PCARenderable) {
                boundingComp.m_PCARenderable->Render(transform.m_Model, viewMatrix, projectionMatrix);
            }
            
            if (m_ShowOBB && boundingComp.m_OBBRenderable) {
                boundingComp.m_OBBRenderable->Render(transform.m_Model, viewMatrix, projectionMatrix);
            }
        }
    }
}

void RenderSystem::Shutdown()
{
    for (auto entity : m_Registry.View<RenderComponent>()) 
    {
        auto& renderComp = m_Registry.GetComponent<RenderComponent>(entity);
        
        // Clean up main renderable
        if (renderComp.m_Renderable) {
            renderComp.m_Renderable->CleanUp();
        }
    }
    
    // Clean up bounding component renderables
    for (auto entity : m_Registry.View<BoundingComponent>()) 
    {
        auto& boundingComp = m_Registry.GetComponent<BoundingComponent>(entity);
        boundingComp.CleanupRenderables();
    }
}

// Bounding volume visibility controls
void RenderSystem::SetShowAABB(bool show)
{
    m_ShowAABB = show;
}

void RenderSystem::SetShowRitterSphere(bool show)
{
    m_ShowRitterSphere = show;
}

void RenderSystem::SetShowLarsonSphere(bool show)
{
    m_ShowLarsonSphere = show;
}

void RenderSystem::SetShowPCASphere(bool show)
{
    m_ShowPCASphere = show;
}

void RenderSystem::SetShowOBB(bool show)
{
    m_ShowOBB = show;
}

bool RenderSystem::IsAABBVisible() const
{
    return m_ShowAABB;
}

bool RenderSystem::IsRitterSphereVisible() const
{
    return m_ShowRitterSphere;
}

bool RenderSystem::IsLarsonSphereVisible() const
{
    return m_ShowLarsonSphere;
}

bool RenderSystem::IsPCASphereVisible() const
{
    return m_ShowPCASphere;
}

bool RenderSystem::IsOBBVisible() const
{
    return m_ShowOBB;
}

// Main object visibility controls
void RenderSystem::SetShowMainObjects(bool show)
{
    m_ShowMainObjects = show;
}

bool RenderSystem::IsShowMainObjects() const
{
    return m_ShowMainObjects;
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
    } 

    Buffer::UpdateUniformBuffer(materialUBO, &material, sizeof(Material));

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
        } 
        Buffer::UpdateUniformBuffer(lightUBO, &light, sizeof(DirectionalLight));

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