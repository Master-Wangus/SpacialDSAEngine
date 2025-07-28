/*
 * @file Octree.hpp
 * @class Octree
 * @brief Adaptive octree spatial partitioning for dynamic 3D scenes.
 *
 * This header declares the Octree class which subdivides space into octants to
 * accelerate spatial operations such as visibility testing, collision
 * detection, and rendering. The tree adapts to changing scene content and can
 * be visualised with CubeRenderers.
 */
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
/**
 * @brief Constructs a new Octree instance.
 * @param registry Reference to the entity registry containing scene objects.
 * @param maxObjectsPerCell Maximum objects allowed in a cell before subdivision.
 * @param method Strategy used to handle objects that straddle multiple child cells.
 * @param maxDepth Maximum subdivision depth of the tree.
 */
    Octree(Registry& registry,
           int                 maxObjectsPerCell = 10,
           StraddlingMethod    method            = StraddlingMethod::UseCenter,
           int                 maxDepth          = 8);

/**
 * @brief Default destructor.
 */
    ~Octree() = default;

/**
 * @brief Rebuilds the octree if it has been marked dirty.
 */
    // Rebuild from scratch (called initially and whenever dirty).
    void Build();

/**
 * @brief Sets the maximum number of objects allowed in a cell and marks tree dirty.
 * @param maxObjects New maximum object count per cell.
 */
    void SetMaxObjectsPerCell(int maxObjects)    { m_MaxObjects = std::max(1, maxObjects); m_Dirty = true; }

/**
 * @brief Sets the strategy for handling straddling objects and marks tree dirty.
 * @param m Strategy to use when an object overlaps multiple child cells.
 */
    void SetStraddlingMethod(StraddlingMethod m) { m_Method = m; m_Dirty = true; }

/**
 * @brief Sets the maximum depth of the octree and marks tree dirty.
 * @param maxDepth New maximum depth.
 */
    void SetMaxDepth(int maxDepth)              { m_MaxDepth = std::max(1, maxDepth); m_Dirty = true; }

/**
 * @brief Gets the current maximum depth of the octree.
 * @return Maximum depth.
 */
    int  GetMaxDepth() const                    { return m_MaxDepth; }

/**
 * @brief Marks the octree as dirty so it will be rebuilt on next access.
 */
    // Flag that we need to rebuild on next access (e.g., after transform changes).
    void MarkDirty() { m_Dirty = true; }

/**
 * @brief Collects CubeRenderer drawables for visualising each octree cell.
 * @param shader Shader to be used when drawing the CubeRenderers.
 * @param out Vector to populate with CubeRenderer instances.
 */
    // Collect CubeRenderer instances for drawing each cell.
    void CollectRenderables(const std::shared_ptr<Shader>& shader,
                            std::vector<std::shared_ptr<CubeRenderer>>& out);

/**
 * @brief Returns a pointer to the root node of the octree.
 * @return Const pointer to the root TreeNode.
 */
    const TreeNode* GetRoot() const;

private:
/**
 * @brief Recursively builds the octree.
 * @param center Centre of the current cell.
 * @param halfWidth Half the side length of the current cell.
 * @param entities Entities contained in this cell.
 * @param level Current recursion depth.
 * @return Unique pointer to constructed TreeNode.
 */
    std::unique_ptr<TreeNode> BuildOctree(const glm::vec3& center, float halfWidth, 
                                                  const std::vector<Registry::Entity>& entities, int level);
    
/**
 * @brief Determines the child index for an object and whether it straddles multiple children.
 * @param pNode Pointer to current node.
 * @param objCenter Centre of the object.
 * @param objExtents Half-extents of the object.
 * @param outIndex Receives the chosen child index (0-7).
 * @param outStraddle Receives true if the object straddles multiple children.
 */
    // Helper functions for object placement
    void GetChildIndex(const TreeNode* pNode,
                       const glm::vec3& objCenter,
                       const glm::vec3& objExtents,
                       int& outIndex,
                       bool& outStraddle);
    
/**
 * @brief Distributes entities to the appropriate child vectors.
 * @param pNode Pointer to current node.
 * @param entities Entities contained in current node.
 * @param childEntities Output array of vectors for each child.
 * @param straddlingEntities Output vector for entities that straddle multiple children.
 */
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