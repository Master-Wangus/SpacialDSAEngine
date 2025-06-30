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
#include <memory>
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

// Add new node type enumeration and pointer-based tree structure -----------------

enum class BvhNodeType { Internal, Leaf };

struct TreeNode
{
    BvhNodeType type = BvhNodeType::Internal;

    // Bounding volumes for this node
    Aabb   aabb;
    Sphere sphere;

    // Leaf data -------------------------------------------------------------
    std::vector<Registry::Entity> objects; // Objects stored in this leaf

    // Hierarchy -------------------------------------------------------------
    TreeNode* parent = nullptr;
    std::unique_ptr<TreeNode> lChild = nullptr;
    std::unique_ptr<TreeNode> rChild = nullptr;

    int depth = 0; // Depth of this node (root = 0)
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

    // Creates wireframe renderables for visualising the BVH. One renderable per
    // node is generated and coloured by depth
    std::vector<std::shared_ptr<IRenderable>>
    CreateRenderables(const std::shared_ptr<Shader>& shader,
                      bool useAabb = true) const;

    // Depth list parallel to CreateRenderables result (root-first traversal)
    const std::vector<int>& GetDepths() const;

    // Map from entity to its leaf node index for quick updates
    std::unordered_map<Entity,TreeNode*> m_EntityToLeaf;

    // Refit the hierarchy after a leaf's underlying object has moved / scaled
    void RefitLeaf(Registry& registry, Entity entity);

    // Utility functions for top-down splitting.
    static int ChooseSplitAxis(const std::vector<glm::vec3>& extents);

private:
    // Helper functions --------------------------------------------------------

    // Compute an AABB containing the supplied objects.
    static Aabb ComputeAabb(Registry& registry, const std::vector<Entity>& objs);

    // Compute a bounding sphere containing supplied objects (simple Ritter's over AABB extents).
    static Sphere ComputeSphereFromAabb(const Aabb& box);

    // Data --------------------------------------------------------------------

    // ---------------------------------------------------------------------
    // New pointer-based hierarchy (used by top-down builder)
    // ---------------------------------------------------------------------
    std::unique_ptr<TreeNode> m_Root = nullptr;            // root of new tree

    // Flat representation produced from m_Root for rendering convenience
    mutable std::vector<int> m_FlatDepths;                 // depth per renderable (parallel to CreateRenderables result)

    // Helper to recursively create renderables from pointer-based tree
    void CollectRenderables(const TreeNode* node,
                            bool useAabb,
                            const std::shared_ptr<Shader>& shader,
                            std::vector<std::shared_ptr<IRenderable>>& out) const;

    // (unused)
}; 