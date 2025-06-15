/**
 * @file ResourceSystem.cpp
 * @brief Implementation of the ResourceSystem class for asset management.
 */

#include "ResourceSystem.hpp"
#include "Shader.hpp"
#include "Buffer.hpp"
#include <random>

ResourceSystem& ResourceSystem::GetInstance() 
{
    static ResourceSystem instance;
    return instance;
}

ResourceSystem::ResourceSystem() 
{
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    m_Loader = std::make_unique<Assimp::Importer>();
}

ResourceSystem::~ResourceSystem() 
{
    Clear();
}

ResourceHandle ResourceSystem::LoadMesh(const std::string& path) 
{
    // Load mesh from file
    auto mesh = LoadOBJFile(path);
    if (!mesh) 
    {
        return INVALID_RESOURCE_HANDLE; // Return invalid handle
    }
    
    // Create new handle with random UUID
    ResourceHandle handle = GenerateRandomUUID();
    
    // Store in cache
    m_MeshResources[handle] = mesh;
    
    return handle;
}

std::shared_ptr<MeshResource> ResourceSystem::GetMesh(const ResourceHandle& handle) const 
{
    if (!IsHandleValid(handle)) 
    {
        return nullptr;
    }
    
    auto it = m_MeshResources.find(handle);
    if (it != m_MeshResources.end()) 
    {
        return it->second;
    }
    
    return nullptr;
}

void ResourceSystem::Clear() 
{
    m_MeshResources.clear();
}

void ResourceSystem::ClearUnused() 
{
    // Remove resources with only one reference (the one in our map)
    for (auto it = m_MeshResources.begin(); it != m_MeshResources.end();) 
    {
        if (it->second.use_count() == 1) 
        {
            // Remove the resource
            it = m_MeshResources.erase(it);
        } 
        else 
        {
            ++it;
        }
    }
}

uint64_t ResourceSystem::GenerateRandomUUID() 
{
    // Generate a random 64-bit UUID
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> dis(1, UINT64_MAX); // Start from 1, 0 is reserved for invalid handles
    
    uint64_t uuid = dis(gen);
    
    // Make sure it's not already in use (highly unlikely but still check)
    while (uuid == INVALID_RESOURCE_HANDLE) // 0 is reserved for invalid handles
    {
        uuid = dis(gen);
    }
    
    return uuid;
}

std::shared_ptr<MeshResource> ResourceSystem::LoadOBJFile(const std::string& path) 
{
    // Optimized post-processing flags - removed expensive operations
    unsigned int flags = aiProcess_Triangulate |            // Ensure all faces are triangles (necessary)
                        aiProcess_JoinIdenticalVertices |   // Optimize mesh (good for performance)
                        aiProcess_ValidateDataStructure;    // Fast validation
    
    // Only add these flags if really needed:
    // aiProcess_GenSmoothNormals - expensive, only if normals missing
    // aiProcess_FlipUVs - only if UV coordinates are flipped
    
    const aiScene* scene = m_Loader->ReadFile(path, flags);
    
    // Check for errors
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) 
    {
        std::cerr << "ERROR::ASSIMP::" << m_Loader->GetErrorString() << std::endl;
        return nullptr;
    }
    
    // Create the mesh resource
    auto meshResource = std::make_shared<MeshResource>();
    std::vector<Vertex> vertices;
    
    // Pre-allocate total vertex count for better performance
    size_t totalVertices = 0;
    for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
        totalVertices += scene->mMeshes[i]->mNumFaces * 3; // 3 vertices per triangle
    }
    vertices.reserve(totalVertices);
    
    // Process all meshes in the scene
    for (unsigned int i = 0; i < scene->mNumMeshes; i++) 
    {
        const aiMesh* mesh = scene->mMeshes[i];
        auto meshVertices = ProcessAssimpMesh(mesh);
        vertices.insert(vertices.end(), 
                       std::make_move_iterator(meshVertices.begin()), 
                       std::make_move_iterator(meshVertices.end()));
    }
    
    if (vertices.empty()) 
    {
        std::cerr << "Failed to load mesh: " << path << std::endl;
        return nullptr;
    }
    
    // Store vertices in the mesh resource
    meshResource->SetVertices(std::move(vertices)); // Use move to avoid copy
    
    return meshResource;
}

std::vector<Vertex> ResourceSystem::ProcessAssimpMesh(const aiMesh* mesh) 
{
    std::vector<Vertex> vertices;
    vertices.reserve(mesh->mNumVertices); // Pre-allocate for better performance
    
    // Process unique vertices once
    for (unsigned int i = 0; i < mesh->mNumVertices; i++) 
    {
        // Get vertex position
        glm::vec3 position(
            mesh->mVertices[i].x,
            mesh->mVertices[i].y,
            mesh->mVertices[i].z
        );
        
        // Get vertex normal (if available)
        glm::vec3 normal(0.0f, 1.0f, 0.0f); // Default normal
        if (mesh->HasNormals()) 
        {
            normal = glm::vec3(
                mesh->mNormals[i].x,
                mesh->mNormals[i].y,
                mesh->mNormals[i].z
            );
        }
        
        // Get texture coordinates (if available)
        glm::vec2 uv(0.0f, 0.0f); // Default UVs
        if (mesh->HasTextureCoords(0)) 
        {
            uv = glm::vec2(
                mesh->mTextureCoords[0][i].x,
                mesh->mTextureCoords[0][i].y
            );
        }
        
        // Get vertex color (if available), otherwise use white
        glm::vec3 color(1.0f, 1.0f, 1.0f); // Default color
        if (mesh->HasVertexColors(0)) 
        {
            color = glm::vec3(
                mesh->mColors[0][i].r,
                mesh->mColors[0][i].g,
                mesh->mColors[0][i].b
            );
        }
        
        // Create and add the vertex
        Vertex vertex = { position, color, normal, uv };
        vertices.push_back(vertex);
    }
    
    // Now expand to triangulated vertices (temporary fix to maintain compatibility)
    std::vector<Vertex> triangulatedVertices;
    triangulatedVertices.reserve(mesh->mNumFaces * 3); // Pre-allocate
    
    for (unsigned int j = 0; j < mesh->mNumFaces; j++) 
    {
        const aiFace& face = mesh->mFaces[j];
        for (unsigned int k = 0; k < face.mNumIndices; k++) 
        {
            unsigned int index = face.mIndices[k];
            triangulatedVertices.push_back(vertices[index]);
        }
    }
    
    return triangulatedVertices;
} 