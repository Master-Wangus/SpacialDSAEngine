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
#include "CameraSystem.hpp"
#include "EventSystem.hpp"
#include "Keybinds.hpp"
#include "Octree.hpp"

RenderSystem::RenderSystem(Registry& registry, Window& window, const std::shared_ptr<Shader>& shader)
    : m_Registry(registry), m_Window(window), m_Shader(shader), m_GlobalWireframe(false)
{
    window.SetFramebufferSizeCallback([](int width, int height)
        {
        glViewport(0, 0, width, height);
    });

    EventSystem::Get().SubscribeToEvent(EventType::KeyPress, [this](const EventData& eventData) 
        {
        if (auto keyCode = std::get_if<int>(&eventData)) 
        {
            if (*keyCode == Keybinds::KEY_F) 
            {
                SetGlobalWireframe(!m_GlobalWireframe);
                std::cout << "Wireframe mode: " << (m_GlobalWireframe ? "ON" : "OFF") << std::endl;
            }
        }
    });

    EventSystem::Get().SubscribeToEvent(EventType::TransformChanged, [this](const EventData& eventData)
        {
            m_OctreeDirty = true;
        });

    EventSystem::Get().SubscribeToEvent(EventType::SceneReset, [this](const EventData&)
        {
            m_OctreeDirty = true;
        });
}

void RenderSystem::BuildOctree()
{
    if (!m_Octree)
    {
        m_Octree = std::make_unique<Octree>(m_Registry, m_OctreeMaxObjects, m_StradMethod, m_OctreeMaxDepth);
    }
    else
    {
        m_Octree->SetMaxObjectsPerCell(m_OctreeMaxObjects);
        m_Octree->SetStraddlingMethod(m_StradMethod);
        m_Octree->SetMaxDepth(m_OctreeMaxDepth);
    }

    m_Octree->MarkDirty(); // ensure rebuild
    m_Octree->Build();

    m_OctreeRenderables.clear();
    m_Octree->CollectRenderables(m_Shader, m_OctreeRenderables);
    m_OctreeDirty = false;
}

void RenderSystem::SetShowOctree(bool show)
{
    m_ShowOctreeCells = show;
    if (show)
        m_OctreeDirty = true;
}

bool RenderSystem::IsOctreeVisible() const { return m_ShowOctreeCells; }

void RenderSystem::SetOctreeMaxObjects(int maxObjects)
{
    m_OctreeMaxObjects = std::max(1, maxObjects);
    m_OctreeDirty = true;
}
int RenderSystem::GetOctreeMaxObjects() const { return m_OctreeMaxObjects; }

void RenderSystem::SetStraddlingMethod(StraddlingMethod method)
{
    if (m_StradMethod != method)
    {
        m_StradMethod = method;
        m_OctreeDirty = true;
    }
}
StraddlingMethod RenderSystem::GetStraddlingMethod() const { return m_StradMethod; }

void RenderSystem::SetOctreeMaxDepth(int depth)
{
    m_OctreeMaxDepth = std::max(1, depth);
    m_OctreeDirty = true;
}

int RenderSystem::GetOctreeMaxDepth() const { return m_OctreeMaxDepth; }


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

    BuildOctree();
}

void RenderSystem::Render()
{
    if (m_LightEntity != entt::null && m_Registry.HasComponent<DirectionalLightComponent>(m_LightEntity))
    {
        auto& lightComp = m_Registry.GetComponent<DirectionalLightComponent>(m_LightEntity);

        float time = static_cast<float>(m_Window.GetTime());
        float angle = time * m_LightRotationSpeed; // radians

        glm::vec3 dir = glm::normalize(glm::vec3(0.0f, -cos(angle), -sin(angle)));
        lightComp.m_Light.m_Direction = glm::vec4(dir, 0.0f);

        if (m_LightVisualizationEntity != entt::null && m_Registry.HasComponent<TransformComponent>(m_LightVisualizationEntity))
        {
            glm::vec3 lightPos = -dir * 5.0f;
            auto& t = m_Registry.GetComponent<TransformComponent>(m_LightVisualizationEntity);
            t.m_Position = lightPos;
            t.UpdateModelMatrix();
        }

        UpdateLighting();
    }

    if (m_OctreeDirty)
    {
        BuildOctree();
    }

    auto cameraView = m_Registry.View<CameraComponent>();
    if (cameraView.empty()) return;
    
    auto cameraEntity = *cameraView.begin();
    auto& camera = m_Registry.GetComponent<CameraComponent>(cameraEntity);
    
    float aspectRatio = static_cast<float>(m_Window.GetWidth()) / static_cast<float>(m_Window.GetHeight());
    glm::mat4 viewMatrix = camera.GetViewMatrix();
    glm::mat4 projectionMatrix = camera.GetProjectionMatrix(aspectRatio);
    
    glm::vec3 cameraPosition = camera.GetPosition();
    
    m_Shader->Use();
    m_Shader->SetVec3("viewPos", cameraPosition);
    
    if (m_CameraSystem) 
    {
        m_CameraSystem->UpdateFrustumPlanes(camera, aspectRatio);
    }
    
    static GLenum s_CurrentPolyMode = GL_FILL;
    GLenum desiredMode = m_GlobalWireframe ? GL_LINE : GL_FILL;
    if (desiredMode != s_CurrentPolyMode)
    {
        glPolygonMode(GL_FRONT_AND_BACK, desiredMode);
        s_CurrentPolyMode = desiredMode;
    }
    
    auto renderView = m_Registry.View<TransformComponent, RenderComponent>();
    for (auto entity : renderView) 
    {
        auto& transform = m_Registry.GetComponent<TransformComponent>(entity);
        auto& renderComp = m_Registry.GetComponent<RenderComponent>(entity);
        
        if (!renderComp.m_IsVisible)
            continue;
            
        if (entity == m_LightVisualizationEntity) 
        {
            if (m_ShowMainObjects && renderComp.m_Renderable) 
            {
                renderComp.m_Renderable->Render(transform.m_Model, viewMatrix, projectionMatrix);
            }
            continue;
        }
        
        SideResult frustumResult = SideResult::eINSIDE;
        
        if (m_CameraSystem && m_Registry.HasComponent<BoundingComponent>(entity)) 
        {
            auto& boundingComp = m_Registry.GetComponent<BoundingComponent>(entity);

            Aabb worldAabb = boundingComp.GetAABB();
            worldAabb.Transform(transform.m_Model);

            Sphere worldPCA    = boundingComp.GetPCASphere();

            auto transformPoint = [&](const glm::vec3& p){ return glm::vec3(transform.m_Model * glm::vec4(p,1.0f)); };
            worldPCA.center = transformPoint(worldPCA.center);

            float maxScale = glm::compMax(glm::abs(transform.m_Scale));
            worldPCA.radius    *= maxScale;

            Obb worldObb = boundingComp.GetOBB();
            worldObb.center = transformPoint(worldObb.center);
            for(int i=0;i<3;++i){
                worldObb.axes[i] = glm::normalize(glm::mat3(transform.m_Model) * worldObb.axes[i]);
                worldObb.halfExtents[i] *= maxScale;
            }

            if (m_ShowAABB) 
            {
                frustumResult = m_CameraSystem->TestAabbAgainstFrustum(worldAabb);
            }
            else if (m_ShowOBB) 
            {
                frustumResult = m_CameraSystem->TestObbAgainstFrustum(worldObb);
            }
            else if (m_ShowPCASphere) 
            {
                frustumResult = m_CameraSystem->TestSphereAgainstFrustum(worldPCA);
            }
        }
        
        if (m_ShowMainObjects && renderComp.m_Renderable) 
        {            
            renderComp.m_Renderable->Render(transform.m_Model, viewMatrix, projectionMatrix);
        }
        
        if (m_Registry.HasComponent<BoundingComponent>(entity))
        {            
            auto& boundingComp = m_Registry.GetComponent<BoundingComponent>(entity);
                        
            if (m_ShowAABB && boundingComp.m_AABBRenderable)
            {
                boundingComp.m_AABBRenderable->Render(transform.m_Model, viewMatrix, projectionMatrix);
            }
            
            if (m_ShowOBB && boundingComp.m_OBBRenderable) 
            {
                boundingComp.m_OBBRenderable->Render(transform.m_Model, viewMatrix, projectionMatrix);
            }

            if (m_ShowPCASphere && boundingComp.m_PCARenderable)
            {
                 boundingComp.m_PCARenderable->Render(transform.m_Model, viewMatrix, projectionMatrix);
            }

            UpdateMaterialUBO(m_DefaultMaterial);
        }
    }

    if (m_ShowOctreeCells)
    {
        for (const auto& cube : m_OctreeRenderables)
        {
            cube->Render(glm::mat4(1.0f), viewMatrix, projectionMatrix);
        }
    }
}

void RenderSystem::Shutdown()
{
    for (auto entity : m_Registry.View<RenderComponent>()) 
    {
        auto& renderComp = m_Registry.GetComponent<RenderComponent>(entity);
        
        if (renderComp.m_Renderable) 
        {
            renderComp.m_Renderable->CleanUp();
        }
    }
    
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

bool RenderSystem::IsPCASphereVisible() const
{
    return m_ShowPCASphere;
}

bool RenderSystem::IsOBBVisible() const
{
    return m_ShowOBB;
}

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
    
    CreateLightSourceVisualization(light);
    
    UpdateLighting();
}

void RenderSystem::CreateLightSourceVisualization(const DirectionalLight& light)
{

    glm::vec3 lightDirection = glm::normalize(glm::vec3(light.m_Direction));
    glm::vec3 lightPosition = -lightDirection * 5.0f; // Place 5 units away
    
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
    m_DefaultMaterial = Material();
    
    m_DefaultMaterial.m_AmbientIntensity  = 0.5f;
    m_DefaultMaterial.m_SpecularIntensity = 0.5f;
    m_DefaultMaterial.m_Shininess         = 32.0f;
    
    if (m_MaterialUBO == 0) 
    {
        m_MaterialUBO = Buffer::CreateUniformBuffer(sizeof(Material), 1);
        
        GLuint materialBlockIndex = glGetUniformBlockIndex(m_Shader->GetID(), "Material");
        if (materialBlockIndex != GL_INVALID_INDEX) {
            glUniformBlockBinding(m_Shader->GetID(), materialBlockIndex, 1);
        } else {
            std::cerr << "ERROR: Material uniform block not found in shader!" << std::endl;
        }
    } 

    Buffer::UpdateUniformBuffer(m_MaterialUBO, &m_DefaultMaterial, sizeof(Material));
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

void RenderSystem::SetGlobalWireframe(bool enabled)
{
    m_GlobalWireframe = enabled;
}

bool RenderSystem::IsGlobalWireframeEnabled() const
{
    return m_GlobalWireframe;
}
