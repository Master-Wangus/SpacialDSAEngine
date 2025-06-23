/**
 * @file MeshRenderer.hpp
 * @brief Renderer for mesh resources with custom transformation.
 *
 * This class implements the IRenderable interface to render 3D meshes loaded as resources.
 */

#pragma once

#include "pch.h"
#include "IRenderable.hpp"
#include "ResourceSystem.hpp"

class MeshRenderer : public IRenderable 
{
public:
    /**
     * @brief Constructs a mesh renderer with a mesh resource handle.
     * @param meshHandle Handle to the mesh resource to render
     */
    explicit MeshRenderer(const ResourceHandle& meshHandle);
    
    /**
     * @brief Constructs a mesh renderer with mesh resource and color.
     * @param meshHandle Handle to the mesh resource to render
     * @param color Color to apply to the mesh
     */
    MeshRenderer(const ResourceHandle& meshHandle, const glm::vec3& color);
    
    /**
     * @brief Constructs a mesh renderer with mesh resource, color, and wireframe option.
     * @param meshHandle Handle to the mesh resource to render
     * @param color Color to apply to the mesh
     * @param wireframe Whether to render in wireframe mode
     */
    MeshRenderer(const ResourceHandle& meshHandle, const glm::vec3& color, bool wireframe);
    
    /**
     * @brief Destructor for the mesh renderer.
     */
    ~MeshRenderer() override;
    
    /**
     * @brief Initializes the mesh renderer with a shader.
     * @param shader Shared pointer to the shader program
     */
    void Initialize(const std::shared_ptr<Shader>& shader) override;
    
    /**
     * @brief Renders the mesh with the given transformation matrices.
     * @param modelMatrix Model transformation matrix
     * @param viewMatrix View transformation matrix
     * @param projectionMatrix Projection transformation matrix
     */
    void Render(const glm::mat4& modelMatrix, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) override;
    
    /**
     * @brief Cleans up OpenGL resources.
     */
    void CleanUp() override;
    
    /**
     * @brief Sets the mesh resource to render.
     * @param meshHandle Handle to the new mesh resource
     */
    void SetMesh(const ResourceHandle& meshHandle);
    
    /**
     * @brief Sets the color of the mesh.
     * @param color New color to apply
     */
    void SetColor(const glm::vec3& color);
    
    /**
     * @brief Gets the current color of the mesh.
     * @return Current mesh color
     */
    glm::vec3 GetColor() const;
    
    /**
     * @brief Sets whether to render in wireframe mode.
     * @param wireframe True for wireframe, false for solid
     */
    void SetWireframe(bool wireframe);
    
    /**
     * @brief Checks if the mesh is rendered in wireframe mode.
     * @return True if wireframe, false if solid
     */
    bool IsWireframe() const;
    
private:
    ResourceHandle m_MeshHandle;
    glm::vec3 m_Color = glm::vec3(1.0f);
    bool m_Initialized = false;
    bool m_Wireframe = false;
    
    /**
     * @brief Updates vertex colors to match current color setting.
     */
    void UpdateVertexColors();
}; 