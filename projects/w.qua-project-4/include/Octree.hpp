#pragma once

#include "pch.h"
#include "Shapes.hpp"
#include "Components.hpp"
#include "Registry.hpp"
#include "CubeRenderer.hpp"
#include <array>
#include <memory>

enum class StraddlingMethod
{
    UseCenter = 0,        // Decide by the object's centre point only.
    StayAtCurrentLevel    // Keep straddling objects at current level instead of subdividing.
};

struct TreeNode
{
    glm::vec3 center;                 // Cell centre
    float     halfwidth;              // Half the side length of the cubic cell
    std::array<std::unique_ptr<TreeNode>, 8> children; // unique_ptr children (nullptr if leaf)
    std::vector<Registry::Entity> pObjects; // Entities contained in this cell
    int level;                  // Depth level in the tree (for coloring)

    TreeNode(const glm::vec3& c, float hw, int lvl = 0)
        : center(c), halfwidth(hw), level(lvl)
    {}
};

class Octree
{
public:
    Octree(Registry& registry,
           int                 maxObjectsPerCell = 10,
           StraddlingMethod    method            = StraddlingMethod::UseCenter,
           int                 maxDepth          = 8);

    ~Octree() = default; 

    // Rebuild from scratch (called initially and whenever dirty).
    void Build();

    void SetMaxObjectsPerCell(int maxObjects)    { m_MaxObjects = std::max(1, maxObjects); m_Dirty = true; }
    void SetStraddlingMethod(StraddlingMethod m) { m_Method = m; m_Dirty = true; }
    void SetMaxDepth(int maxDepth)              { m_MaxDepth = std::max(1, maxDepth); m_Dirty = true; }
    int  GetMaxDepth() const                    { return m_MaxDepth; }

    // Flag that we need to rebuild on next access (e.g., after transform changes).
    void MarkDirty() { m_Dirty = true; }

    // Collect CubeRenderer instances for drawing each cell.
    void CollectRenderables(const std::shared_ptr<Shader>& shader,
                            std::vector<std::shared_ptr<CubeRenderer>>& out);

    const TreeNode* GetRoot() const;

private:
    std::unique_ptr<TreeNode> BuildOctree(const glm::vec3& center, float halfWidth, 
                                                  const std::vector<Registry::Entity>& entities, int level);
    
    // Helper functions for object placement
    void GetChildIndex(const TreeNode* pNode,
                       const glm::vec3& objCenter,
                       const glm::vec3& objExtents,
                       int& outIndex,
                       bool& outStraddle);
    
    void DistributeObjectsToChildren(const TreeNode* pNode,
                                     const std::vector<Registry::Entity>& entities,
                                     std::vector<Registry::Entity> childEntities[8],
                                     std::vector<Registry::Entity>& straddlingEntities);

    Registry&            m_Registry;
    std::unique_ptr<TreeNode> m_Root;

    int                  m_MaxObjects;
    StraddlingMethod     m_Method;
    int                  m_MaxDepth;  

    bool                 m_Dirty = true;
}; 