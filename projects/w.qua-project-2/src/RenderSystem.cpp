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
#include "FrustumRenderer.hpp"
#include "Components.hpp"
#include "Shapes.hpp"
#include "Registry.hpp"
#include "Window.hpp"
#include "Buffer.hpp"
#include "Lighting.hpp"
#include "CameraSystem.hpp"

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

    // Initialize frustum renderer
    m_FrustumRenderer = std::make_shared<FrustumRenderer>(glm::vec3(1.0f, 0.0f, 1.0f)); // Magenta color
    m_FrustumRenderer->Initialize(m_Shader);

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
    
    // Update frustum planes for culling
    if (m_CameraSystem) {
        m_CameraSystem->UpdateFrustumPlanes(camera, aspectRatio);
    }
    
    // Set polygon mode for main objects to always be wireframe
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    
    // Render entities with both transform and render components
    auto renderView = m_Registry.View<TransformComponent, RenderComponent>();
    for (auto entity : renderView) 
    {
        auto& transform = m_Registry.GetComponent<TransformComponent>(entity);
        auto& renderComp = m_Registry.GetComponent<RenderComponent>(entity);
        
        // Skip if not visible
        if (!renderComp.m_IsVisible)
            continue;
            
        // Skip light entity for frustum culling
        if (entity == m_LightVisualizationEntity) {
            // Always render the light visualization
            if (m_ShowMainObjects && renderComp.m_Renderable) {
                renderComp.m_Renderable->Render(transform.m_Model, viewMatrix, projectionMatrix);
            }
            continue;
        }
        
        // Apply frustum culling if enabled
        bool isVisible = true;
        SideResult frustumResult = SideResult::eINSIDE;
        
        if (m_CameraSystem && m_Registry.HasComponent<BoundingComponent>(entity)) 
        {
            auto& boundingComp = m_Registry.GetComponent<BoundingComponent>(entity);
            
            if (m_ShowAABB) 
            {
                frustumResult = m_CameraSystem->TestAabbAgainstFrustum(boundingComp.m_AABB);
            }
            else if (m_ShowOBB) 
            {
                frustumResult = m_CameraSystem->TestObbAgainstFrustum(boundingComp.m_OBB);
            }
            else if (m_ShowRitterSphere) 
            {
                frustumResult = m_CameraSystem->TestSphereAgainstFrustum(boundingComp.m_RitterSphere);
            }
            else if (m_ShowLarsonSphere) 
            {
                frustumResult = m_CameraSystem->TestSphereAgainstFrustum(boundingComp.m_LarssonSphere);
            }
            else if (m_ShowPCASphere) 
            {
                frustumResult = m_CameraSystem->TestSphereAgainstFrustum(boundingComp.m_PCASphere);
            }
        }
        
        // Only render if visible (or culling is disabled)
        if (m_ShowMainObjects && renderComp.m_Renderable) 
        {
            // Disable lighting for wireframe main objects to show pure colors
            m_Shader->SetBool("disableLighting", true);
            
            // Render with original material - no color changes based on intersection
            renderComp.m_Renderable->Render(transform.m_Model, viewMatrix, projectionMatrix);
        }
        
        // Render bounding volumes if enabled and entity has BoundingComponent
        if (m_Registry.HasComponent<BoundingComponent>(entity))
        {
            // Reset to solid mode for bounding volume wireframes 
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            
            // Enable lighting for bounding volume objects (they are not wireframes)
            m_Shader->SetBool("disableLighting", false);
            
            auto& boundingComp = m_Registry.GetComponent<BoundingComponent>(entity);
                        
            if (m_ShowAABB && boundingComp.m_AABBRenderable && m_CameraSystem) 
            {
                SideResult aabbResult = m_CameraSystem->TestAabbAgainstFrustum(boundingComp.m_AABB);
                Material& aabbMaterial = boundingComp.m_AABBRenderable->GetMaterialEditable();
                glm::vec3 originalAabbDiffuse = aabbMaterial.m_DiffuseColor;
                glm::vec3 originalAabbAmbient = aabbMaterial.m_AmbientColor;
                glm::vec3 originalAabbSpecular = aabbMaterial.m_SpecularColor;
                
                glm::vec3 aabbTestColor = m_CameraSystem->GetFrustumTestColor(aabbResult);
                aabbMaterial.m_DiffuseColor = aabbTestColor;
                aabbMaterial.m_AmbientColor = aabbTestColor;
                aabbMaterial.m_SpecularColor = aabbTestColor;
                
                // Update the material UBO with modified values
                UpdateMaterialUBO(aabbMaterial);
                
                boundingComp.m_AABBRenderable->Render(transform.m_Model, viewMatrix, projectionMatrix);
                
                // Restore original colors
                aabbMaterial.m_DiffuseColor = originalAabbDiffuse;
                aabbMaterial.m_AmbientColor = originalAabbAmbient;
                aabbMaterial.m_SpecularColor = originalAabbSpecular;
                
                // Restore original material UBO
                UpdateMaterialUBO(aabbMaterial);
            }
            
            if (m_ShowRitterSphere && boundingComp.m_RitterRenderable && m_CameraSystem) 
            {
                SideResult ritterResult = m_CameraSystem->TestSphereAgainstFrustum(boundingComp.m_RitterSphere);
                Material& ritterMaterial = boundingComp.m_RitterRenderable->GetMaterialEditable();
                glm::vec3 originalRitterDiffuse = ritterMaterial.m_DiffuseColor;
                glm::vec3 originalRitterAmbient = ritterMaterial.m_AmbientColor;
                glm::vec3 originalRitterSpecular = ritterMaterial.m_SpecularColor;
                
                glm::vec3 ritterTestColor = m_CameraSystem->GetFrustumTestColor(ritterResult);
                ritterMaterial.m_DiffuseColor = ritterTestColor;
                ritterMaterial.m_AmbientColor = ritterTestColor;
                ritterMaterial.m_SpecularColor = ritterTestColor;
                
                // Update the material UBO with modified values
                UpdateMaterialUBO(ritterMaterial);
                
                boundingComp.m_RitterRenderable->Render(transform.m_Model, viewMatrix, projectionMatrix);
                
                // Restore original colors
                ritterMaterial.m_DiffuseColor = originalRitterDiffuse;
                ritterMaterial.m_AmbientColor = originalRitterAmbient;
                ritterMaterial.m_SpecularColor = originalRitterSpecular;
                
                // Restore original material UBO
                UpdateMaterialUBO(ritterMaterial);
            }
            
            if (m_ShowLarsonSphere && boundingComp.m_LarsonRenderable && m_CameraSystem) 
            {
                SideResult larsonResult = m_CameraSystem->TestSphereAgainstFrustum(boundingComp.m_LarssonSphere);
                Material& larsonMaterial = boundingComp.m_LarsonRenderable->GetMaterialEditable();
                glm::vec3 originalLarsonDiffuse = larsonMaterial.m_DiffuseColor;
                glm::vec3 originalLarsonAmbient = larsonMaterial.m_AmbientColor;
                glm::vec3 originalLarsonSpecular = larsonMaterial.m_SpecularColor;
                
                glm::vec3 larsonTestColor = m_CameraSystem->GetFrustumTestColor(larsonResult);
                larsonMaterial.m_DiffuseColor = larsonTestColor;
                larsonMaterial.m_AmbientColor = larsonTestColor;
                larsonMaterial.m_SpecularColor = larsonTestColor;
                
                // Update the material UBO with modified values
                UpdateMaterialUBO(larsonMaterial);
                
                boundingComp.m_LarsonRenderable->Render(transform.m_Model, viewMatrix, projectionMatrix);
                
                // Restore original colors
                larsonMaterial.m_DiffuseColor = originalLarsonDiffuse;
                larsonMaterial.m_AmbientColor = originalLarsonAmbient;
                larsonMaterial.m_SpecularColor = originalLarsonSpecular;
                
                // Restore original material UBO
                UpdateMaterialUBO(larsonMaterial);
            }
            
            if (m_ShowPCASphere && boundingComp.m_PCARenderable && m_CameraSystem) 
            {
                SideResult pcaResult = m_CameraSystem->TestSphereAgainstFrustum(boundingComp.m_PCASphere);
                Material& pcaMaterial = boundingComp.m_PCARenderable->GetMaterialEditable();
                glm::vec3 originalPcaDiffuse = pcaMaterial.m_DiffuseColor;
                glm::vec3 originalPcaAmbient = pcaMaterial.m_AmbientColor;
                glm::vec3 originalPcaSpecular = pcaMaterial.m_SpecularColor;
                
                glm::vec3 pcaTestColor = m_CameraSystem->GetFrustumTestColor(pcaResult);
                pcaMaterial.m_DiffuseColor = pcaTestColor;
                pcaMaterial.m_AmbientColor = pcaTestColor;
                pcaMaterial.m_SpecularColor = pcaTestColor;
                
                // Update the material UBO with modified values
                UpdateMaterialUBO(pcaMaterial);
                
                boundingComp.m_PCARenderable->Render(transform.m_Model, viewMatrix, projectionMatrix);
                
                // Restore original colors
                pcaMaterial.m_DiffuseColor = originalPcaDiffuse;
                pcaMaterial.m_AmbientColor = originalPcaAmbient;
                pcaMaterial.m_SpecularColor = originalPcaSpecular;
                
                // Restore original material UBO
                UpdateMaterialUBO(pcaMaterial);
            }
            
            if (m_ShowOBB && boundingComp.m_OBBRenderable && m_CameraSystem) 
            {
                SideResult obbResult = m_CameraSystem->TestObbAgainstFrustum(boundingComp.m_OBB);
                Material& obbMaterial = boundingComp.m_OBBRenderable->GetMaterialEditable();
                glm::vec3 originalObbDiffuse = obbMaterial.m_DiffuseColor;
                glm::vec3 originalObbAmbient = obbMaterial.m_AmbientColor;
                glm::vec3 originalObbSpecular = obbMaterial.m_SpecularColor;
                
                glm::vec3 obbTestColor = m_CameraSystem->GetFrustumTestColor(obbResult);
                obbMaterial.m_DiffuseColor = obbTestColor;
                obbMaterial.m_AmbientColor = obbTestColor;
                obbMaterial.m_SpecularColor = obbTestColor;
                
                // Update the material UBO with modified values
                UpdateMaterialUBO(obbMaterial);
                
                boundingComp.m_OBBRenderable->Render(transform.m_Model, viewMatrix, projectionMatrix);
                
                // Restore original colors
                obbMaterial.m_DiffuseColor = originalObbDiffuse;
                obbMaterial.m_AmbientColor = originalObbAmbient;
                obbMaterial.m_SpecularColor = originalObbSpecular;
                
                // Restore original material UBO
                UpdateMaterialUBO(obbMaterial);
            }
        }
    }
    
    // Render frustum visualization if enabled
    if (m_ShowFrustum && m_FrustumRenderer && m_CameraSystem) {
        // Get the inverse view-projection matrix for frustum visualization
        float aspectRatio = static_cast<float>(m_Window.GetWidth()) / static_cast<float>(m_Window.GetHeight());
        glm::mat4 viewProjection = m_CameraSystem->GetVisualizationViewProjectionMatrix(camera, aspectRatio);
        glm::mat4 invViewProjection = glm::inverse(viewProjection);
        
        // Update frustum geometry
        m_FrustumRenderer->UpdateFrustum(invViewProjection);
        
        // Render frustum
        glm::mat4 identity(1.0f);
        m_FrustumRenderer->Render(identity, viewMatrix, projectionMatrix);
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
    
    if (m_MaterialUBO == 0) 
    {
        m_MaterialUBO = Buffer::CreateUniformBuffer(sizeof(Material), 1);
        
        // Bind the Material uniform block to binding point 1
        GLuint materialBlockIndex = glGetUniformBlockIndex(m_Shader->GetID(), "Material");
        if (materialBlockIndex != GL_INVALID_INDEX) {
            glUniformBlockBinding(m_Shader->GetID(), materialBlockIndex, 1);
        } else {
            std::cerr << "ERROR: Material uniform block not found in shader!" << std::endl;
        }
    } 

    Buffer::UpdateUniformBuffer(m_MaterialUBO, &material, sizeof(Material));
}

void RenderSystem::UpdateMaterialUBO(const Material& material)
{
    if (m_MaterialUBO != 0) {
        Buffer::UpdateUniformBuffer(m_MaterialUBO, &material, sizeof(Material));
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
            
            // Bind the DirectionalLight uniform block to binding point 0
            GLuint lightBlockIndex = glGetUniformBlockIndex(m_Shader->GetID(), "DirectionalLight");
            if (lightBlockIndex != GL_INVALID_INDEX) {
                glUniformBlockBinding(m_Shader->GetID(), lightBlockIndex, 0);
            } else {
                std::cerr << "ERROR: DirectionalLight uniform block not found in shader!" << std::endl;
            }
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

void RenderSystem::EnableFrustumCulling(bool enable)
{
    m_EnableFrustumCulling = enable;
}

bool RenderSystem::IsFrustumCullingEnabled() const
{
    return m_EnableFrustumCulling;
}

void RenderSystem::SetCameraSystem(CameraSystem* cameraSystem)
{
    m_CameraSystem = cameraSystem;
}

// Frustum visualization controls
void RenderSystem::SetShowFrustum(bool show)
{
    m_ShowFrustum = show;
}

bool RenderSystem::IsShowFrustum() const
{
    return m_ShowFrustum;
} 