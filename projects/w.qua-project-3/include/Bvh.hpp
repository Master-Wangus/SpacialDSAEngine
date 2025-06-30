/*
 * @file Bvh.hpp
 * @brief Bounding Volume Hierarchy (BVH) structures and builders for Axis-Aligned Bounding Boxes (AABB) and Bounding Spheres.
 *
 * The BVH implementation supports:
 *   • Top-down construction with multiple split strategies.
 *   • Bottom-up construction with several merge heuristics.
 *   • Multiple leaf termination criteria.
 *
 * Each node stores both an AABB and a bounding sphere so that the same hierarchy
 * can be visualised with either volume type.
 */
#pragma once

#include "pch.h"
#include "Shapes.hpp"
#include "Registry.hpp"
#include "Components.hpp"
#include <unordered_map>

class IRenderable;
class Shader;

// ──────────────────────────────────────────────────────────────────────────────
// Enumerations controlling BVH construction
// ──────────────────────────────────────────────────────────────────────────────

// Which high-level algorithm to use.
enum class BvhBuildMethod
{
    TopDown,
    BottomUp
};

// Splitting strategies for the top-down builder.
enum class TDSSplitStrategy
{
    MedianCenter,   // Split on median of BV centres along chosen axis
    MedianExtent,   // Split on median of BV extents along chosen axis
    KEven           // Even k-way split on chosen axis (k = 2 by default)
};

// Termination criteria for creating leaf nodes in the top-down builder.
enum class TDSTermination
{
    SingleObject,   // Stop when only one object remains
    TwoObjects,     // Stop when one or two objects remain
    MaxHeight2      // Stop when the current depth is 2 (height 2 tree)
};

// Heuristics for merging in the bottom-up builder.
enum class BUSHeuristic
{
    NearestCenter,          // Merge nodes with nearest centre distance
    MinCombinedVolume,      // Merge nodes that minimise combined AABB volume
    MinCombinedSurfaceArea  // Merge nodes that minimise combined AABB surface area
};

// ──────────────────────────────────────────────────────────────────────────────
// Global configuration (modifiable by UI)
// ──────────────────────────────────────────────────────────────────────────────

struct BvhBuildConfig
{
    // Current build method and parameters selected by the UI. These are used
    // by RenderSystem when it needs to rebuild the BVH automatically (e.g.
    // after transforms change) so that the hierarchy is regenerated with the
    // most recent user preferences.

    static BvhBuildMethod     s_Method;
    static TDSSplitStrategy   s_TDStrategy;
    static TDSTermination     s_TDTermination;
    static BUSHeuristic       s_BUHeuristic;
    static bool               s_UseAabbVisual;
};

// ──────────────────────────────────────────────────────────────────────────────
// BVH Node definition
// ──────────────────────────────────────────────────────────────────────────────

struct BvhNode
{
    // Bounding volumes encompassing this node
    Aabb   m_AABB;
    Sphere m_Sphere;

    // Children indices (-1 if leaf)
    int m_Left  = -1;
    int m_Right = -1;
    int m_Parent = -1;

    std::vector<Registry::Entity> m_Objects; // Non-empty only for leaf nodes

    int m_Depth = 0; // Root = 0

    [[nodiscard]] bool IsLeaf() const { return m_Left == -1 && m_Right == -1; }
};

// ──────────────────────────────────────────────────────────────────────────────
// BVH Builder
// ──────────────────────────────────────────────────────────────────────────────

class Bvh
{
public:
    using Entity = Registry::Entity;

    Bvh() = default;

    // Build a top-down BVH.
    void BuildTopDown(Registry& registry,
                      const std::vector<Entity>& objects,
                      TDSSplitStrategy strategy = TDSSplitStrategy::MedianCenter,
                      TDSTermination termination = TDSTermination::SingleObject,
                      size_t maxHeight = 2);

    // Build a bottom-up BVH.
    void BuildBottomUp(Registry& registry,
                       const std::vector<Entity>& objects,
                       BUSHeuristic heuristic = BUSHeuristic::NearestCenter);

    // Clear hierarchy.
    void Clear();

    const std::vector<BvhNode>& GetNodes() const { return m_Nodes; }
    int  GetRootIndex() const { return m_RootIndex; }

    // Creates wireframe renderables for visualising the BVH. One renderable per
    // node is generated and coloured by depth
    std::vector<std::shared_ptr<IRenderable>>
    CreateRenderables(const std::shared_ptr<Shader>& shader,
                      bool useAabb = true) const;

    // Map from entity to its leaf node index for quick updates
    std::unordered_map<Entity,int> m_EntityToLeaf;

    // Refit the hierarchy after a leaf's underlying object has moved / scaled
    void RefitLeaf(Registry& registry, Entity entity);

private:
    // Helper functions --------------------------------------------------------

    // Compute an AABB containing the supplied objects.
    static Aabb ComputeAabb(Registry& registry, const std::vector<Entity>& objs);

    // Compute a bounding sphere containing supplied objects (simple Ritter's over AABB extents).
    static Sphere ComputeSphereFromAabb(const Aabb& box);

    // Top-down recursive subdivision.
    int BuildTopDownRecursive(Registry& registry,
                              std::vector<Entity>& objects,
                              int depth,
                              TDSSplitStrategy strategy,
                              TDSTermination termination,
                              size_t maxHeight);

    // Utility functions for top-down splitting.
    static int ChooseSplitAxis(const std::vector<glm::vec3>& extents);

    // Bottom-up helpers.
    struct PairKey { int a; int b; };

    float PairCost(const BvhNode& a, const BvhNode& b, BUSHeuristic h) const;

    // Data --------------------------------------------------------------------
    std::vector<BvhNode> m_Nodes;
    int m_RootIndex = -1;
}; 