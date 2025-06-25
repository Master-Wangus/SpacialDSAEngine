/**
 * @class RenderSystem
 * @brief System for rendering 3D objects and scenes.
 *
 * This system manages the rendering process of 3D objects in the scene,
 * including camera view, projection setup, and proper draw call ordering.
 */

#pragma once

#include "pch.h"
#include "Lighting.hpp"
#include "Bvh.hpp"
class Shader;
class Window;
class CameraSystem;

struct RenderComponent;
struct TransformComponent;
struct CameraComponent;
struct DirectionalLightComponent;
struct BoundingComponent;
struct Aabb;
struct Sphere;

class RenderSystem 
{
public:
    /**
     * @brief Constructs the render system with necessary dependencies.
     * @param registry Entity registry for managing render components
     * @param window Window reference for rendering context
     * @param shader Shared pointer to the shader program
     */
    RenderSystem(Registry& registry, Window& window, const std::shared_ptr<Shader>& shader);
    
    /**
     * @brief Initializes the render system and OpenGL resources.
     */
    void Initialize();
    
    /**
     * @brief Renders all visible objects in the scene.
     */
    void Render();
    
    /**
     * @brief Shuts down the render system and cleans up resources.
     */
    void Shutdown();
    
    /**
     * @brief Gets the shader used by the render system.
     * @return Shared pointer to the shader program
     */
    std::shared_ptr<Shader> GetShader() const { return m_Shader; }
    
    // Lighting control
    /**
     * @brief Toggles the directional light on or off.
     * @param enabled True to enable the light, false to disable
     */
    void ToggleDirectionalLight(bool enabled);
    
    /**
     * @brief Checks if the directional light is enabled.
     * @return True if the directional light is enabled, false otherwise
     */
    bool IsDirectionalLightEnabled() const;
    
    /**
     * @brief Updates light direction from visualization position.
     */
    void UpdateLightFromVisualization();
    
    /**
     * @brief Toggles global wireframe mode for all rendered objects.
     * @param enabled True to enable wireframe, false for solid rendering
     */
    void SetGlobalWireframe(bool enabled);
    
    /**
     * @brief Checks if global wireframe mode is enabled.
     * @return True if global wireframe is enabled, false otherwise
     */
    bool IsGlobalWireframeEnabled() const;
    
    /**
     * @brief Gets the entity used for light visualization.
     * @return Entity ID of the light visualization object
     */
    Registry::Entity GetLightVisualizationEntity() const { return m_LightVisualizationEntity; }
    
    // Bounding volume visibility controls
    /**
     * @brief Sets visibility of AABB bounding volumes.
     * @param show True to show AABBs, false to hide
     */
    void SetShowAABB(bool show);
    
    /**
     * @brief Sets visibility of Ritter sphere bounding volumes.
     * @param show True to show Ritter spheres, false to hide
     */
    void SetShowRitterSphere(bool show);
    
    /**
     * @brief Sets visibility of Larson sphere bounding volumes.
     * @param show True to show Larson spheres, false to hide
     */
    void SetShowLarsonSphere(bool show);
    
    /**
     * @brief Sets visibility of PCA sphere bounding volumes.
     * @param show True to show PCA spheres, false to hide
     */
    void SetShowPCASphere(bool show);
    
    /**
     * @brief Sets visibility of OBB bounding volumes.
     * @param show True to show OBBs, false to hide
     */
    void SetShowOBB(bool show);
    
    /**
     * @brief Checks if AABB bounding volumes are visible.
     * @return True if AABBs are visible, false otherwise
     */
    bool IsAABBVisible() const;
    
    /**
     * @brief Checks if Ritter sphere bounding volumes are visible.
     * @return True if Ritter spheres are visible, false otherwise
     */
    bool IsRitterSphereVisible() const;
    
    /**
     * @brief Checks if Larson sphere bounding volumes are visible.
     * @return True if Larson spheres are visible, false otherwise
     */
    bool IsLarsonSphereVisible() const;
    
    /**
     * @brief Checks if PCA sphere bounding volumes are visible.
     * @return True if PCA spheres are visible, false otherwise
     */
    bool IsPCASphereVisible() const;
    
    /**
     * @brief Checks if OBB bounding volumes are visible.
     * @return True if OBBs are visible, false otherwise
     */
    bool IsOBBVisible() const;
    
    // Main object visibility controls
    /**
     * @brief Sets visibility of main scene objects.
     * @param show True to show main objects, false to hide
     */
    void SetShowMainObjects(bool show);
    
    /**
     * @brief Checks if main scene objects are visible.
     * @return True if main objects are visible, false otherwise
     */
    bool IsShowMainObjects() const;
    
    // Frustum culling controls
    /**
     * @brief Enables or disables frustum culling.
     * @param enable True to enable frustum culling, false to disable
     */
    void EnableFrustumCulling(bool enable);
    
    /**
     * @brief Checks if frustum culling is enabled.
     * @return True if frustum culling is enabled, false otherwise
     */
    bool IsFrustumCullingEnabled() const;
    
    /**
     * @brief Sets the camera system for frustum culling calculations.
     * @param cameraSystem Pointer to the camera system
     */
    void SetCameraSystem(CameraSystem* cameraSystem);
    
    // Frustum visualization controls
    /**
     * @brief Sets visibility of the camera frustum visualization.
     * @param show True to show frustum, false to hide
     */
    void SetShowFrustum(bool show);
    
    /**
     * @brief Checks if the camera frustum visualization is visible.
     * @return True if frustum is visible, false otherwise
     */
    bool IsShowFrustum() const;

    // BVH controls -----------------------------------------------------------
    void BuildBVH(BvhBuildMethod method = BvhBuildMethod::TopDown,
                  TDSSplitStrategy tdStrategy = TDSSplitStrategy::MedianCenter,
                  TDSTermination tdTermination = TDSTermination::SingleObject,
                  BUSHeuristic buHeuristic = BUSHeuristic::NearestCenter,
                  bool useAabbVisual = true);

    static constexpr int kMaxBVHLevels = 8;
    void SetBVHLevelVisible(int level, bool visible);
    bool IsBVHLevelVisible(int level) const;
    const std::array<bool, kMaxBVHLevels>& GetBVHLevelMask() const { return m_BvhLevelVisible; }

    void MarkBVHDirty() { m_BvhDirty = true; }

private:
    /**
     * @brief Sets up lighting system and uniform buffer objects.
     */
    void SetupLighting();
    
    /**
     * @brief Updates lighting uniforms for the current frame.
     */
    void UpdateLighting();
    
    /**
     * @brief Sets up material properties for rendering.
     */
    void SetupMaterial();
    
    /**
     * @brief Updates the material uniform buffer object.
     * @param material Material properties to upload
     */
    void UpdateMaterialUBO(const Material& material);
    
    /**
     * @brief Creates a visual representation of the light source.
     * @param light Directional light to visualize
     */
    void CreateLightSourceVisualization(const struct DirectionalLight& light);
    
    Registry& m_Registry;
    Window& m_Window;
    std::shared_ptr<Shader> m_Shader;
    Registry::Entity m_LightEntity = entt::null;
    Registry::Entity m_LightVisualizationEntity = entt::null;
    
    // Bounding volume visibility flags
    bool m_ShowAABB = false;
    bool m_ShowRitterSphere = false;
    bool m_ShowLarsonSphere = false;
    bool m_ShowPCASphere = false;
    bool m_ShowOBB = false;
    
    // Main object display control
    bool m_ShowMainObjects = true;
    
    // Frustum culling control
    bool m_EnableFrustumCulling = false;
    CameraSystem* m_CameraSystem = nullptr;
    
    // Frustum visualization flag retained (no renderer instance)
    bool m_ShowFrustum = false;
    
    // OpenGL buffer IDs
    GLuint m_MaterialUBO = 0;
    
    // Default material used for regular objects; reapplied after bounding-volume draws
    Material m_DefaultMaterial;
    
    // Global wireframe toggle
    bool m_GlobalWireframe = false;

    // BVH controls
    bool m_UseAabbForBVH = true;
    std::unique_ptr<Bvh> m_Bvh;
    std::vector<std::shared_ptr<IRenderable>> m_BvhRenderables;
    std::array<bool, kMaxBVHLevels> m_BvhLevelVisible { { true,false,false,false,false,false,false,false } };
    std::vector<int> m_BvhRenderableDepths;

    bool m_BvhDirty = true;
}; 