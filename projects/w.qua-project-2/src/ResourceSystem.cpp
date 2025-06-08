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
    // Post-processing flags
    unsigned int flags = aiProcess_Triangulate |        // Ensure all faces are triangles
                        aiProcess_GenSmoothNormals |   // Generate normals if not present
                        aiProcess_FlipUVs |            // Flip UV coordinates
                        aiProcess_JoinIdenticalVertices;  // Optimize mesh    
    // Load the OBJ file
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
    
    // Process all meshes in the scene
    for (unsigned int i = 0; i < scene->mNumMeshes; i++) 
    {
        const aiMesh* mesh = scene->mMeshes[i];
        auto meshVertices = ProcessAssimpMesh(mesh);
        vertices.insert(vertices.end(), meshVertices.begin(), meshVertices.end());
    }
    
    if (vertices.empty()) 
    {
        std::cerr << "Failed to load mesh: " << path << std::endl;
        return nullptr;
    }
    
    // Store vertices in the mesh resource
    meshResource->SetVertices(vertices);
    
    return meshResource;
}

std::vector<Vertex> ResourceSystem::ProcessAssimpMesh(const aiMesh* mesh) 
{
    std::vector<Vertex> vertices;
    
    // Process each face (triangle)
    for (unsigned int j = 0; j < mesh->mNumFaces; j++) 
    {
        const aiFace& face = mesh->mFaces[j];
        
        // Process each vertex in the face
        for (unsigned int k = 0; k < face.mNumIndices; k++) 
        {
            unsigned int index = face.mIndices[k];
            
            // Get vertex position
            glm::vec3 position(
                mesh->mVertices[index].x,
                mesh->mVertices[index].y,
                mesh->mVertices[index].z
            );
            
            // Get vertex normal (if available)
            glm::vec3 normal(0.0f, 1.0f, 0.0f); // Default normal
            if (mesh->HasNormals()) 
            {
                normal = glm::vec3(
                    mesh->mNormals[index].x,
                    mesh->mNormals[index].y,
                    mesh->mNormals[index].z
                );
            }
            
            // Get texture coordinates (if available)
            glm::vec2 uv(0.0f, 0.0f); // Default UVs
            if (mesh->HasTextureCoords(0)) 
            {
                uv = glm::vec2(
                    mesh->mTextureCoords[0][index].x,
                    mesh->mTextureCoords[0][index].y
                );
            }
            
            // Get vertex color (if available), otherwise use white
            glm::vec3 color(1.0f, 1.0f, 1.0f); // Default color
            if (mesh->HasVertexColors(0)) 
            {
                color = glm::vec3(
                    mesh->mColors[0][index].r,
                    mesh->mColors[0][index].g,
                    mesh->mColors[0][index].b
                );
            }
            
            // Create and add the vertex
            Vertex vertex = { position, color, normal, uv };
            vertices.push_back(vertex);
        }
    }
    
    return vertices;
} 