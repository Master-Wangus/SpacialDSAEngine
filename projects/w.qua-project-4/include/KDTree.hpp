/*
 * @file KDTree.hpp
 * @class KDTree
 * @brief KD-Tree spatial partitioning structure for efficient scene queries.
 *
 * This header declares the KDTree class which builds and maintains a k-d tree
 * over scene entities stored in an ECS registry. The tree can be rebuilt on
 * demand and provides helpers to visualise the partitions using CubeRenderers.
 */
#pragma once

#include "pch.h"
#include "Shapes.hpp"
#include "Components.hpp"
#include "Registry.hpp"
#include "CubeRenderer.hpp"

// Split strategies for KD-Tree
enum class KdSplitMethod
{
    MedianCenter = 0,   // Split at median of object centres along axis
    MedianExtent = 1,   // Split at median of object extents along axis (size)
    EvenSplit    = 2    // k-even split (not yet implemented)
};

struct KdNode
{
    Aabb bounds;                       // Axis-aligned bounding box of this node
    std::unique_ptr<KdNode> left;      // Child with values < split (nullptr if leaf)
    std::unique_ptr<KdNode> right;     // Child with values >= split
    std::vector<Registry::Entity> objects; // Entities stored in this node (leaf)

    int   level  = 0;     // Depth in tree – used for colouring
    int   axis   = 0;     // Axis this node splits on (0=X,1=Y,2=Z)
    float split  = 0.0f;  // Split position along axis (world units)

    KdNode(const Aabb& b, int lvl = 0) : bounds(b), level(lvl) {}
};

class KDTree
{
public:
/**
 * @brief Constructs a new KDTree instance.
 * @param registry Reference to the entity registry containing objects to partition.
 * @param maxObjectsPerNode Maximum objects allowed in a leaf node before splitting.
 * @param splitMethod Split plane selection strategy.
 * @param maxDepth Maximum recursion depth of the tree.
 */
KDTree(Registry& registry,
       int              maxObjectsPerNode = 10,
       KdSplitMethod    splitMethod       = KdSplitMethod::MedianCenter,
       int              maxDepth          = 32);

/**
 * @brief Default destructor.
 */
~KDTree() = default;

/**
 * @brief Rebuilds the tree if marked dirty.
 */
void Build();

/**
 * @brief Collects CubeRenderer drawables for visualising each node.
 * @param shader Shader to be used when drawing the CubeRenderers.
 * @param out Vector to populate with CubeRenderer instances.
 */
void CollectRenderables(const std::shared_ptr<Shader>& shader,
                        std::vector<std::shared_ptr<CubeRenderer>>& out);

/**
 * @brief Sets the maximum number of objects allowed in a leaf node and marks tree dirty.
 * @param maxObjs New maximum object count per node.
 */
void SetMaxObjectsPerNode(int maxObjs)      { m_MaxObjects = std::max(1, maxObjs); m_Dirty = true; }

/**
 * @brief Sets the method used to choose split planes and marks tree dirty.
 * @param method Split method to use.
 */
void SetSplitMethod(KdSplitMethod method)   { m_SplitMethod = method; m_Dirty = true; }

/**
 * @brief Sets the maximum depth of the tree and marks tree dirty.
 * @param depth New maximum depth.
 */
void SetMaxDepth(int depth)                 { m_MaxDepth = std::max(1, depth); m_Dirty = true; }

/**
 * @brief Gets the current maximum depth of the tree.
 * @return Maximum depth.
 */
int  GetMaxDepth() const                    { return m_MaxDepth; }

/**
 * @brief Marks the tree as dirty so it will be rebuilt on next access.
 */
void MarkDirty() { m_Dirty = true; }

/**
 * @brief Returns a pointer to the root node of the tree.
 * @return Const pointer to KdNode root.
 */
const KdNode* GetRoot() const { return m_Root.get(); }

private:
/**
 * @brief Recursively builds the k-d tree.
 * @param entities List of entities to partition.
 * @param bounds Bounding box containing all entities.
 * @param level Current recursion depth.
 * @return Unique pointer to constructed node.
 */
std::unique_ptr<KdNode> BuildKdTree(const std::vector<Registry::Entity>& entities,
                                    const Aabb& bounds,
                                    int level);

/**
 * @brief Determines the split position along the given axis according to current strategy.
 * @param entities Entities to examine for split.
 * @param axis Axis index (0 = X, 1 = Y, 2 = Z).
 * @return World-space coordinate of the split plane.
 */
float ChooseSplitPosition(const std::vector<Registry::Entity>& entities,
                          int axis);

    Registry&                  m_Registry;
    std::unique_ptr<KdNode>    m_Root;

    int                        m_MaxObjects;
    KdSplitMethod              m_SplitMethod;
    int                        m_MaxDepth;

    bool                       m_Dirty = true;
}; 