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
    
    // Initialize with vertex data
    void SetVertices(const std::vector<Vertex>& vertices) { m_Vertices = vertices; }
    
    // Getter
    const std::vector<Vertex>& GetVertices() const { return m_Vertices; }
    
private:
    std::vector<Vertex> m_Vertices;   // Vertex data
};

class ResourceSystem 
{
public:
    // Singleton access
    static ResourceSystem& GetInstance();
    
    // Delete copy and move constructors and operators
    ResourceSystem(const ResourceSystem&) = delete;
    ResourceSystem& operator=(const ResourceSystem&) = delete;
    ResourceSystem(ResourceSystem&&) = delete;
    ResourceSystem& operator=(ResourceSystem&&) = delete;
    
    // Mesh resource methods
    ResourceHandle LoadMesh(const std::string& path);
    std::shared_ptr<MeshResource> GetMesh(const ResourceHandle& handle) const;
    
    // Resource management
    void Clear();
    void ClearUnused();
    
    // Helper function to check if a handle is valid
    static bool IsHandleValid(const ResourceHandle& handle) 
    {
        return handle != INVALID_RESOURCE_HANDLE;
    }
    
private:
    ResourceSystem();  // Private constructor for singleton
    ~ResourceSystem();
    
    // Resource cache
    std::unordered_map<ResourceHandle, std::shared_ptr<MeshResource>> m_MeshResources;
    
    // Assimp importer for mesh loading
    std::unique_ptr<Assimp::Importer> m_Loader;
    
    // Generate a random UUID for resource handles
    uint64_t GenerateRandomUUID();
    
    // Resource loading helpers
    std::shared_ptr<MeshResource> LoadOBJFile(const std::string& path);
    std::vector<Vertex> ProcessAssimpMesh(const aiMesh* mesh);
}; 