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
#include "Bvh.hpp"
#include <array>

RenderSystem::RenderSystem(Registry& registry, Window& window, const std::shared_ptr<Shader>& shader)
    : m_Registry(registry), m_Window(window), m_Shader(shader), m_GlobalWireframe(false)
{
    window.SetFramebufferSizeCallback([](int width, int height)
        {
        glViewport(0, 0, width, height);
    });

    // Subscribe to wireframe toggle events
    EventSystem::Get().SubscribeToEvent(EventType::KeyPress, [this](const EventData& eventData) {
        if (auto keyCode = std::get_if<int>(&eventData)) {
            if (*keyCode == Keybinds::KEY_F) {
                SetGlobalWireframe(!m_GlobalWireframe);
                std::cout << "Wireframe mode: " << (m_GlobalWireframe ? "ON" : "OFF") << std::endl;
            }
        }
    });

    // Listen for transform changes to rebuild BVH automatically
    EventSystem::Get().SubscribeToEvent(EventType::TransformChanged, [this](const EventData& eventData) {
        if (m_Bvh)
        {
            if (auto entPtr = std::get_if<entt::entity>(&eventData))
            {
                m_Bvh->RefitLeaf(m_Registry, *entPtr);
                // Visualisation BVs are now stale; simplest is to mark dirty for rebuild of renderables only
                m_BvhDirty = true;
            }
            else
            {
                m_BvhDirty = true;
            }
        }
        else
        {
            m_BvhDirty = true;
        }
    });

    // Rebuild BVH when the entire scene is reset
    EventSystem::Get().SubscribeToEvent(EventType::SceneReset, [this](const EventData&) {
        // Clear any previous hierarchy so we don't reference destroyed entities
        m_Bvh.reset();
        m_BvhDirty = true;
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
    // Rotate directional light vertically over time -----------------------
    if (m_LightEntity != entt::null && m_Registry.HasComponent<DirectionalLightComponent>(m_LightEntity))
    {
        auto& lightComp = m_Registry.GetComponent<DirectionalLightComponent>(m_LightEntity);

        // Compute new direction based on elapsed time
        float time = static_cast<float>(m_Window.GetTime());
        float angle = time * m_LightRotationSpeed; // radians

        // Rotate in the YZ plane (vertical swing)
        glm::vec3 dir = glm::normalize(glm::vec3(0.0f, -cos(angle), -sin(angle)));
        lightComp.m_Light.m_Direction = glm::vec4(dir, 0.0f);

        // Move the visualization sphere opposite the light direction
        if (m_LightVisualizationEntity != entt::null && m_Registry.HasComponent<TransformComponent>(m_LightVisualizationEntity))
        {
            glm::vec3 lightPos = -dir * 5.0f;
            auto& t = m_Registry.GetComponent<TransformComponent>(m_LightVisualizationEntity);
            t.m_Position = lightPos;
            t.UpdateModelMatrix();
        }

        // Upload to GPU UBO
        UpdateLighting();
    }

    // Rebuild BVH automatically if marked dirty (e.g., transforms changed)
    if (m_BvhDirty)
    {
        // Rebuild using current settings chosen in ImGui 
        BuildBVH(BvhBuildConfig::s_Method,
                 BvhBuildConfig::s_TDStrategy,
                 BvhBuildConfig::s_TDTermination,
                 BvhBuildConfig::s_BUHeuristic,
                 BvhBuildConfig::s_UseAabbVisual);
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
    
    // Update frustum planes for culling
    if (m_CameraSystem) 
    {
        m_CameraSystem->UpdateFrustumPlanes(camera, aspectRatio);
    }
    
    // Apply global wireframe mode once per frame
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

            // Compute world-space versions
            Aabb worldAabb = boundingComp.GetAABB();
            worldAabb.Transform(transform.m_Model);

            Sphere worldPCA    = boundingComp.GetPCASphere();

            // Transform sphere centers
            auto transformPoint = [&](const glm::vec3& p){ return glm::vec3(transform.m_Model * glm::vec4(p,1.0f)); };
            worldPCA.center = transformPoint(worldPCA.center);

            // Scale radii by maximum scale factor (uniform approximation)
            float maxScale = glm::compMax(glm::abs(transform.m_Scale));
            worldPCA.radius    *= maxScale;

            Obb worldObb = boundingComp.GetOBB();
            // Transform OBB center & axes
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
            // Lighting is always enabled now.
            
            renderComp.m_Renderable->Render(transform.m_Model, viewMatrix, projectionMatrix);
        }
        
        if (m_Registry.HasComponent<BoundingComponent>(entity))
        {
            // Bounding volumes rendered with lighting enabled.
            
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

            // Reapply default material so subsequent objects use correct shading.
            UpdateMaterialUBO(m_DefaultMaterial);
        }
    }

    // ───── Draw BVH hierarchy (optional) ─────────────────────────────────────
    if (!m_BvhRenderables.empty())
    {
        glm::mat4 identity(1.0f);
        for (size_t i = 0; i < m_BvhRenderables.size(); ++i)
        {
            int depth = (i < m_BvhRenderableDepths.size()) ? m_BvhRenderableDepths[i] : 0;
            if (depth < kMaxBVHLevels && !m_BvhLevelVisible[depth])
                continue;

            if (m_BvhRenderables[i])
                m_BvhRenderables[i]->Render(identity, viewMatrix, projectionMatrix);
        }
    }
}

void RenderSystem::Shutdown()
{
    for (auto entity : m_Registry.View<RenderComponent>()) 
    {
        auto& renderComp = m_Registry.GetComponent<RenderComponent>(entity);
        
        // Clean up main renderable
        if (renderComp.m_Renderable) 
        {
            renderComp.m_Renderable->CleanUp();
        }
    }
    
    // Clean up bounding component renderables
    for (auto entity : m_Registry.View<BoundingComponent>()) 
    {
        auto& boundingComp = m_Registry.GetComponent<BoundingComponent>(entity);
        boundingComp.CleanupRenderables();
    }

    // Clean up any previous visualisation
    for (auto& r : m_BvhRenderables)
    {
        if (r) r->CleanUp();
    }
    m_BvhRenderables.clear();

    m_BvhRenderableDepths.clear();
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
    m_DefaultMaterial = Material();
    
    // Slightly toned-down default values
    m_DefaultMaterial.m_AmbientIntensity  = 0.5f;
    m_DefaultMaterial.m_SpecularIntensity = 0.5f;
    m_DefaultMaterial.m_Shininess         = 32.0f;
    
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

void RenderSystem::SetGlobalWireframe(bool enabled)
{
    m_GlobalWireframe = enabled;
}

bool RenderSystem::IsGlobalWireframeEnabled() const
{
    return m_GlobalWireframe;
}

// ─────────────────────────────────────────────────────────────────────────────
// BVH level mask setters/getters
// ─────────────────────────────────────────────────────────────────────────────

void RenderSystem::SetBVHLevelVisible(int level, bool visible)
{
    if (level >=0 && level < kMaxBVHLevels)
        m_BvhLevelVisible[level] = visible;
}

bool RenderSystem::IsBVHLevelVisible(int level) const
{
    if (level>=0 && level<kMaxBVHLevels) return m_BvhLevelVisible[level];
    return true;
}

// ──────────────────────────────────────────────────────────────────────────────
// BVH functionality
// ──────────────────────────────────────────────────────────────────────────────

void RenderSystem::BuildBVH(BvhBuildMethod method,
                            TDSSplitStrategy tdStrategy,
                            TDSTermination tdTermination,
                            BUSHeuristic buHeuristic,
                            bool useAabbVisual)
{
    // Clean up old BVH visualisation
    for (auto& r : m_BvhRenderables)
    {
        if (r) r->CleanUp();
    }
    m_BvhRenderables.clear();

    // Collect all entities that have bounding components
    std::vector<Registry::Entity> entities;
    auto view = m_Registry.View<BoundingComponent>();
    for (auto e : view) entities.push_back(e);

    if (entities.empty()) return;

    m_Bvh = std::make_unique<Bvh>();

    if (method == BvhBuildMethod::TopDown)
    {
        m_Bvh->BuildTopDown(m_Registry, entities, tdStrategy, tdTermination);
    }
    else
    {
        m_Bvh->BuildBottomUp(m_Registry, entities, buHeuristic);
    }

    m_BvhRenderables = m_Bvh->CreateRenderables(m_Shader, useAabbVisual);
    m_UseAabbForBVH = useAabbVisual;

    // Store depth for each renderable (same order as nodes)
    m_BvhRenderableDepths.clear();
    for (const auto& node : m_Bvh->GetNodes())
    {
        m_BvhRenderableDepths.push_back(node.m_Depth);
    }

    // BVH up-to-date
    m_BvhDirty = false;
} 