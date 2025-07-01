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

enum class BvhNodeType { Internal, Leaf };

struct TreeNode
{
    BvhNodeType type = BvhNodeType::Internal;

    // Bounding volumes for this node
    Aabb   aabb;
    Sphere sphere;

    // Leaf data 
    std::vector<Registry::Entity> objects; // Objects stored in this leaf

    // Hierarchy 
    TreeNode* parent = nullptr;
    std::unique_ptr<TreeNode> lChild = nullptr;
    std::unique_ptr<TreeNode> rChild = nullptr;

    int depth = 0; // Depth of this node (root = 0)
};


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
    static bool               s_BuildWithAabb;
};


class Bvh
{
public:
    using Entity = Registry::Entity;

    Bvh() = default;

    /**
     * @brief Builds a Bounding Volume Hierarchy using a recursive top-down (splitting) strategy.
     *
     * The input vector is not modified; a copy is made internally so that the
     * algorithm can reorder objects in place while partitioning. The resulting
     * hierarchy root is stored in @c m_Root and the entity-to-leaf cache is
     * refreshed.
     *
     * @param registry   Reference to the ECS registry used to fetch bounding and
     *                   transform components for the supplied entities.
     * @param objects    Set of entities that will become the leaves of the
     *                   hierarchy.
     * @param strategy   Splitting strategy that decides which axis and median is
     *                   used when partitioning each node (default:
     *                   TDSSplitStrategy::MedianCenter).
     * @param termination Leaf-creation rule that stops recursion when satisfied
     *                    (default: TDSTermination::SingleObject).
     * @param maxHeight  Additional upper bound on tree depth used only when the
     *                   termination mode is MaxHeight2. Ignored otherwise.
     */
    void BuildTopDown(Registry& registry,
                      const std::vector<Entity>& objects,
                      TDSSplitStrategy strategy = TDSSplitStrategy::MedianCenter,
                      TDSTermination termination = TDSTermination::SingleObject,
                      size_t maxHeight = 2);

    /**
     * @brief Builds a Bounding Volume Hierarchy bottom-up by greedily merging the
     *        pair of nodes that minimises a user-selected heuristic.
     *
     * Each entity starts as a separate leaf; the algorithm then iteratively
     * creates parents until exactly one root remains. Suitable for small scenes
     * or expensive offline builds.
     *
     * @param registry  Reference to the ECS registry for component look-ups.
     * @param objects   List of entities to include in the hierarchy.
     * @param heuristic Merge heuristic that scores every candidate pair (default:
     *                  BUSHeuristic::NearestCenter).
     */
    void BuildBottomUp(Registry& registry,
                       const std::vector<Entity>& objects,
                       BUSHeuristic heuristic = BUSHeuristic::NearestCenter);

    /**
     * @brief Destroys the current hierarchy and clears all auxiliary caches.
     *
     * After the call @c m_Root becomes @c nullptr, @c m_FlatDepths is emptied and
     * the @c m_EntityToLeaf map is cleared.
     */
    void Clear();


    /**
     * @brief Generates one wire-frame cube or sphere per BVH node for visual
     *        debugging.
     *
     * The traversal order is root-first (pre-order) so the returned vector is
     * parallel to @ref GetDepths(). Colours cycle through a fixed palette based
     * on node depth.
     *
     * @param shader   Shader to be assigned to each generated renderable.
     * @param useAabb  When true AABBs are rendered; when false PCA spheres are
     *                 rendered instead.
     * @return Vector of @c IRenderable shared pointers, one for each node.
     */
    std::vector<std::shared_ptr<IRenderable>>
    CreateRenderables(const std::shared_ptr<Shader>& shader,
                      bool useAabb = true) const;

    /**
     * @brief Returns the depth, in tree levels, of every node returned by the last
     *        call to @ref CreateRenderables().
     */
    const std::vector<int>& GetDepths() const;

    // Map from entity to its leaf node index for quick updates
    std::unordered_map<Entity,TreeNode*> m_EntityToLeaf;

    /**
     * @brief Updates the bounding volumes of the path from a modified leaf up to
     *        the root.
     *
     * Call this after changing an entity's transform or geometry so that the BVH
     * remains conservative without a full rebuild.
     *
     * @param registry ECS registry used to fetch updated component data.
     * @param entity   The entity whose leaf node should be refitted.
     */
    void RefitLeaf(Registry& registry, Entity entity);

    /**
     * @brief Returns the index of the axis with the greatest variance in the given
     *        set of 3-D vectors.
     *
     * Used by the top-down splitter to pick the most "spread-out" dimension.
     *
     * @param extents Collection of extents or centres.
     * @return 0 for x, 1 for y, 2 for z.
     */
    static int ChooseSplitAxis(const std::vector<glm::vec3>& extents);

private:
    /**
     * @brief Computes a world-space axis-aligned bounding box that encloses all
     *        entities in the supplied vector.
     *
     * @param registry Registry for component access.
     * @param objs     Entities to aggregate.
     * @return Combined Aabb in world coordinates.
     */
    static Aabb ComputeAabb(Registry& registry, const std::vector<Entity>& objs);

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