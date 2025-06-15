/**
 * @file ResourceSystem.hpp
 * @brief System for managing and retrieving various asset types.
 *
 * This system manages the loading, caching, and retrieval of various resource types
 * including meshes, textures, and other assets needed by the application.
 */

#pragma once

#include "pch.h"

// Forward declarations
class Shader;
struct Vertex;
namespace Assimp 
{
    class Importer;
}

// Simple typedef for ResourceHandle - just a 64-bit UUID
typedef uint64_t ResourceHandle;

// Invalid handle constant
const ResourceHandle INVALID_RESOURCE_HANDLE = 0;

// MeshResource class (now contained within ResourceSystem.hpp)
class MeshResource 
{
public:
    MeshResource() = default;
    ~MeshResource() = default;
    
    /**
     * @brief Sets the vertex data for this mesh resource.
     * @param vertices Vector of vertex data to store
     */
    void SetVertices(const std::vector<Vertex>& vertices) { m_Vertices = vertices; }
    
    /**
     * @brief Sets the vertex data for this mesh resource (move version).
     * @param vertices Vector of vertex data to move
     */
    void SetVertices(std::vector<Vertex>&& vertices) { m_Vertices = std::move(vertices); }
    
    /**
     * @brief Gets the vertex data for this mesh resource.
     * @return Const reference to the vertex data
     */
    const std::vector<Vertex>& GetVertexes() const { return m_Vertices; }
    
private:
    std::vector<Vertex> m_Vertices;   // Vertex data
};

class ResourceSystem 
{
public:
    /**
     * @brief Gets the singleton instance of the resource system.
     * @return Reference to the singleton instance
     */
    static ResourceSystem& GetInstance();
    
    // Delete copy and move constructors and operators
    ResourceSystem(const ResourceSystem&) = delete;
    ResourceSystem& operator=(const ResourceSystem&) = delete;
    ResourceSystem(ResourceSystem&&) = delete;
    ResourceSystem& operator=(ResourceSystem&&) = delete;
    
    // Mesh resource methods
    /**
     * @brief Loads a mesh from file and returns a handle to it.
     * @param path File path to the mesh resource
     * @return Handle to the loaded mesh resource
     */
    ResourceHandle LoadMesh(const std::string& path);
    
    /**
     * @brief Gets a mesh resource by its handle.
     * @param handle Handle to the mesh resource
     * @return Shared pointer to the mesh resource, or nullptr if not found
     */
    std::shared_ptr<MeshResource> GetMesh(const ResourceHandle& handle) const;
    
    // Resource management
    /**
     * @brief Clears all cached resources from memory.
     */
    void Clear();
    
    /**
     * @brief Clears unused resources from memory.
     */
    void ClearUnused();
    
    /**
     * @brief Checks if a resource handle is valid.
     * @param handle Resource handle to check
     * @return True if handle is valid, false otherwise
     */
    static bool IsHandleValid(const ResourceHandle& handle) 
    {
        return handle != INVALID_RESOURCE_HANDLE;
    }
    
private:
    /**
     * @brief Private constructor for singleton pattern.
     */
    ResourceSystem();
    
    /**
     * @brief Private destructor for singleton pattern.
     */
    ~ResourceSystem();
    
    // Resource cache
    std::unordered_map<ResourceHandle, std::shared_ptr<MeshResource>> m_MeshResources;
    
    // Assimp importer for mesh loading
    std::unique_ptr<Assimp::Importer> m_Loader;
    
    /**
     * @brief Generates a random UUID for resource handles.
     * @return Generated UUID as 64-bit integer
     */
    uint64_t GenerateRandomUUID();
    
    /**
     * @brief Loads an OBJ file and creates a mesh resource.
     * @param path Path to the OBJ file
     * @return Shared pointer to the loaded mesh resource
     */
    std::shared_ptr<MeshResource> LoadOBJFile(const std::string& path);
    
    /**
     * @brief Processes an Assimp mesh and converts to vertex data.
     * @param mesh Assimp mesh to process
     * @return Vector of processed vertex data
     */
    std::vector<Vertex> ProcessAssimpMesh(const aiMesh* mesh);
}; 