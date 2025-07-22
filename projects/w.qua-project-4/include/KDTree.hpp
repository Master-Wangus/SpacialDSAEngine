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
    KDTree(Registry& registry,
           int              maxObjectsPerNode = 10,
           KdSplitMethod    splitMethod       = KdSplitMethod::MedianCenter,
           int              maxDepth          = 32);

    ~KDTree() = default;

    // Rebuild tree on demand
    void Build();

    // Render-helper: produce CubeRenderers for each node bounding box
    void CollectRenderables(const std::shared_ptr<Shader>& shader,
                            std::vector<std::shared_ptr<CubeRenderer>>& out);

    void SetMaxObjectsPerNode(int maxObjs)      { m_MaxObjects = std::max(1, maxObjs); m_Dirty = true; }
    void SetSplitMethod(KdSplitMethod method)   { m_SplitMethod = method; m_Dirty = true; }
    void SetMaxDepth(int depth)                 { m_MaxDepth = std::max(1, depth); m_Dirty = true; }
    int  GetMaxDepth() const                    { return m_MaxDepth; }

    // Mark tree dirty externally (e.g., after transforms changed)
    void MarkDirty() { m_Dirty = true; }

    const KdNode* GetRoot() const { return m_Root.get(); }

private:
    std::unique_ptr<KdNode> BuildKdTree(const std::vector<Registry::Entity>& entities,
                                           const Aabb& bounds,
                                           int level);

    float ChooseSplitPosition(const std::vector<Registry::Entity>& entities,
                              int axis);

    Registry&                  m_Registry;
    std::unique_ptr<KdNode>    m_Root;

    int                        m_MaxObjects;
    KdSplitMethod              m_SplitMethod;
    int                        m_MaxDepth;

    bool                       m_Dirty = true;
}; 