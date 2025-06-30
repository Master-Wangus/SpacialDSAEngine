#include "Bvh.hpp"
#include <queue>
#include <numeric>
#include "CubeRenderer.hpp"
#include "SphereRenderer.hpp"
#include "Shader.hpp"
#include <functional>
#include <algorithm>

// Forward declaration
static std::unique_ptr<TreeNode> BuildTopDownTree(Registry& registry,
                                                 Registry::Entity* objects,
                                                 int numObjects,
                                                 int depth,
                                                 TDSSplitStrategy strategy,
                                                 TDSTermination termination,
                                                 TreeNode* parent = nullptr);

// ──────────────────────────────────────────────────────────────────────────────
// Utility helpers (anonymous namespace)
// ──────────────────────────────────────────────────────────────────────────────

namespace
{
    inline float Volume(const Aabb& a)
    {
        glm::vec3 ext = a.max - a.min;
        return ext.x * ext.y * ext.z;
    }

    inline float SurfaceArea(const Aabb& a)
    {
        glm::vec3 ext = a.max - a.min;
        return 2.0f * (ext.x * ext.y + ext.y * ext.z + ext.x * ext.z);
    }

    inline Aabb Union(const Aabb& a, const Aabb& b)
    {
        return { glm::min(a.min, b.min), glm::max(a.max, b.max) };
    }

    // Non-member utility: compute AABB of a contiguous entity array (global scope)
    static Aabb ComputeAabbRange(Registry& registry, const Registry::Entity* objs, int count)
    {
        if (count <= 0) return {};

        auto makeWorldAabb = [&](Registry::Entity e){
            auto aabb = registry.GetComponent<BoundingComponent>(e).GetAABB();
            if (registry.HasComponent<TransformComponent>(e))
            {
                aabb.Transform(registry.GetComponent<TransformComponent>(e).m_Model);
            }
            return aabb;
        };

        Aabb first = makeWorldAabb(objs[0]);
        glm::vec3 mn = first.min;
        glm::vec3 mx = first.max;

        for (int i = 1; i < count; ++i)
        {
            Aabb aabb = makeWorldAabb(objs[i]);
            mn = glm::min(mn, aabb.min);
            mx = glm::max(mx, aabb.max);
        }
        return { mn, mx };
    }
}

// ──────────────────────────────────────────────────────────────────────────────
// Static configuration defaults
// ──────────────────────────────────────────────────────────────────────────────

BvhBuildMethod   BvhBuildConfig::s_Method        = BvhBuildMethod::TopDown;
TDSSplitStrategy BvhBuildConfig::s_TDStrategy    = TDSSplitStrategy::MedianCenter;
TDSTermination   BvhBuildConfig::s_TDTermination = TDSTermination::SingleObject;
BUSHeuristic     BvhBuildConfig::s_BUHeuristic   = BUSHeuristic::MinCombinedVolume;
bool             BvhBuildConfig::s_UseAabbVisual = true;

// ──────────────────────────────────────────────────────────────────────────────
// Public interface
// ──────────────────────────────────────────────────────────────────────────────

void Bvh::Clear()
{
    m_Root.reset();
    m_FlatDepths.clear();
    m_EntityToLeaf.clear();
}

void Bvh::BuildTopDown(Registry& registry,
                        const std::vector<Entity>& objects,
                        TDSSplitStrategy strategy,
                        TDSTermination termination,
                        size_t /*maxHeight*/)
{
    Clear();
    if (objects.empty()) return;

    // Make a mutable copy so we can partition in-place with nth_element
    std::vector<Entity> objs = objects;

    // Build the recursive pointer-based hierarchy
    m_Root = BuildTopDownTree(registry,
                              objs.data(),
                              static_cast<int>(objs.size()),
                              0,
                              strategy,
                              termination);

    // ------------------------------------------------------------
    // Populate entity → leaf map & flat depth list for rendering
    // ------------------------------------------------------------
    std::function<void(TreeNode*)> traverse = [&](TreeNode* node){
        if (!node) return;
        if (node->type == BvhNodeType::Leaf)
        {
            for (const auto& e : node->objects)
            {
                m_EntityToLeaf[e] = node;
            }
        }
        traverse(node->lChild.get());
        traverse(node->rChild.get());
    };
    traverse(m_Root.get());
}

void Bvh::BuildBottomUp(Registry& registry,
                        const std::vector<Entity>& objects,
                        BUSHeuristic heuristic)
{
    Clear();
    if (objects.empty()) return;

    // Active list owns its nodes via unique_ptr
    std::vector<std::unique_ptr<TreeNode>> active;
    active.reserve(objects.size());

    for (auto entity : objects)
    {
        auto leaf = std::make_unique<TreeNode>();
        leaf->type = BvhNodeType::Leaf;
        leaf->objects.push_back(entity);
        leaf->aabb   = ComputeAabbRange(registry, &entity, 1);
        leaf->sphere = ComputeSphereFromAabb(leaf->aabb);
        leaf->depth  = 0;

        m_EntityToLeaf[entity] = leaf.get();

        active.push_back(std::move(leaf));
    }

    auto pairCost = [&](const TreeNode* a, const TreeNode* b){
        switch (heuristic)
        {
            case BUSHeuristic::NearestCenter:
                return glm::distance(a->aabb.GetCenter(), b->aabb.GetCenter());
            case BUSHeuristic::MinCombinedVolume:
            {
                Aabb u = Union(a->aabb, b->aabb);
                glm::vec3 ext = u.max - u.min;
                return ext.x * ext.y * ext.z;
            }
            case BUSHeuristic::MinCombinedSurfaceArea:
            default:
            {
                Aabb u = Union(a->aabb, b->aabb);
                glm::vec3 ext = u.max - u.min;
                return 2.f * (ext.x*ext.y + ext.y*ext.z + ext.x*ext.z);
            }
        }
    };

    // Greedy merge until one root remains
    while (active.size() > 1)
    {
        float bestCost = std::numeric_limits<float>::max();
        size_t bestI = 0, bestJ = 1;

        for (size_t i = 0; i < active.size(); ++i)
        {
            for (size_t j = i + 1; j < active.size(); ++j)
            {
                float c = pairCost(active[i].get(), active[j].get());
                if (c < bestCost)
                {
                    bestCost = c;
                    bestI = i; bestJ = j;
                }
            }
        }

        // Move out the unique_ptrs for the selected pair
        auto leftUP  = std::move(active[bestI]);
        auto rightUP = std::move(active[bestJ]);

        TreeNode* left  = leftUP.get();
        TreeNode* right = rightUP.get();

        auto parent = std::make_unique<TreeNode>();
        parent->type = BvhNodeType::Internal;
        parent->lChild = std::move(leftUP);
        parent->rChild = std::move(rightUP);
        parent->lChild->parent = parent.get();
        parent->rChild->parent = parent.get();
        parent->depth = std::max(left->depth, right->depth) + 1;
        parent->aabb  = Union(left->aabb, right->aabb);
        parent->sphere = ComputeSphereFromAabb(parent->aabb);

        // Remove pair indices (ensure higher index first)
        if (bestI > bestJ) std::swap(bestI, bestJ);
        active.erase(active.begin() + bestJ);
        active.erase(active.begin() + bestI);

        // Append the new parent node
        active.push_back(std::move(parent));
    }

    // Transfer the last remaining unique_ptr as the root
    m_Root = std::move(active.front());
}

void Bvh::RefitLeaf(Registry& registry, Entity entity)
{
    auto it = m_EntityToLeaf.find(entity);
    if (it == m_EntityToLeaf.end()) return;

    TreeNode* leaf = it->second;
    if (!leaf) return;

    // Recompute leaf bounds
    if (!leaf->objects.empty())
        leaf->aabb = ComputeAabbRange(registry, leaf->objects.data(), static_cast<int>(leaf->objects.size()));
    leaf->sphere = ComputeSphereFromAabb(leaf->aabb);

    // Walk up the tree and update internal nodes
    for (TreeNode* node = leaf->parent; node; node = node->parent)
    {
        if (node->lChild && node->rChild)
            node->aabb = Union(node->lChild->aabb, node->rChild->aabb);
        else if (node->lChild)
            node->aabb = node->lChild->aabb;
        else if (node->rChild)
            node->aabb = node->rChild->aabb;

        node->sphere = ComputeSphereFromAabb(node->aabb);
    }
}

// ──────────────────────────────────────────────────────────────────────────────
// Private helpers
// ──────────────────────────────────────────────────────────────────────────────

Aabb Bvh::ComputeAabb(Registry& registry, const std::vector<Entity>& objs)
{
    if (objs.empty()) return {};

    auto makeWorldAabb = [&](Entity e){
        auto aabb = registry.GetComponent<BoundingComponent>(e).GetAABB();
        if (registry.HasComponent<TransformComponent>(e))
        {
            const auto& model = registry.GetComponent<TransformComponent>(e).m_Model;
            aabb.Transform(model);
        }
        return aabb;
    };

    Aabb firstAabb = makeWorldAabb(objs.front());
    glm::vec3 min = firstAabb.min;
    glm::vec3 max = firstAabb.max;

    for (size_t i = 1; i < objs.size(); ++i)
    {
        Aabb aabb = makeWorldAabb(objs[i]);
        min = glm::min(min, aabb.min);
        max = glm::max(max, aabb.max);
    }
    return { min, max };
}

Sphere Bvh::ComputeSphereFromAabb(const Aabb& box)
{
    glm::vec3 centre = box.GetCenter();
    float rad = glm::length(box.GetExtents());
    return { centre, rad };
}

int Bvh::ChooseSplitAxis(const std::vector<glm::vec3>& extents)
{
    if (extents.empty()) return 0;
    float meanX=0, meanY=0, meanZ=0;
    for (const auto& v : extents)
    {
        meanX += v.x; meanY += v.y; meanZ += v.z;
    }
    meanX /= extents.size();
    meanY /= extents.size();
    meanZ /= extents.size();

    float varX=0, varY=0, varZ=0;
    for (const auto& v : extents)
    {
        varX += (v.x - meanX)*(v.x - meanX);
        varY += (v.y - meanY)*(v.y - meanY);
        varZ += (v.z - meanZ)*(v.z - meanZ);
    }

    if (varX >= varY && varX >= varZ) return 0;
    if (varY >= varZ) return 1;
    return 2;
}

std::vector<std::shared_ptr<IRenderable>>
Bvh::CreateRenderables(const std::shared_ptr<Shader>& shader, bool useAabb) const
{
    std::vector<std::shared_ptr<IRenderable>> result;
    m_FlatDepths.clear();

    if (!shader || !m_Root) return result;

    // Traverse pointer-based tree
    CollectRenderables(m_Root.get(), useAabb, shader, result);
    return result;
}

// Helper to recursively create renderables from pointer-based tree
void Bvh::CollectRenderables(const TreeNode* node,
                             bool useAabb,
                             const std::shared_ptr<Shader>& shader,
                             std::vector<std::shared_ptr<IRenderable>>& out) const
{
    if (!node) return;

    static const glm::vec3 palette[7] = {
        {1.0f, 0.0f, 0.0f},
        {1.0f, 0.5f, 0.0f},
        {1.0f, 1.0f, 0.0f},
        {0.0f, 1.0f, 0.0f},
        {0.0f, 0.0f, 1.0f},
        {0.3f, 0.0f, 0.5f},
        {0.6f, 0.0f, 1.0f}
    };

    glm::vec3 colour = palette[node->depth % 7];

    if (useAabb)
    {
        glm::vec3 size = node->aabb.max - node->aabb.min;
        glm::vec3 centre = node->aabb.GetCenter();
        auto cube = std::make_shared<CubeRenderer>(centre, size, colour, true);
        cube->Initialize(shader);
        out.push_back(cube);
    }
    else
    {
        auto sphere = std::make_shared<SphereRenderer>(node->sphere.center, node->sphere.radius, colour, true);
        sphere->Initialize(shader);
        out.push_back(sphere);
    }

    // record depth parallel to out
    m_FlatDepths.push_back(node->depth);

    CollectRenderables(node->lChild.get(), useAabb, shader, out);
    CollectRenderables(node->rChild.get(), useAabb, shader, out);
}

const std::vector<int>& Bvh::GetDepths() const
{
    return m_FlatDepths;
}

// ─────────────────────────────────────────────────────────────────────────────
// Pointer-based top-down helpers (new implementation)
// ─────────────────────────────────────────────────────────────────────────────

namespace // anonymous
{
    // Forward declarations
    using Entity = Registry::Entity;

    // ComputeAabbRange defined earlier at global scope

    static int PartitionObjects(Registry& registry,
                                Entity* objects,
                                int numObjects,
                                TDSSplitStrategy strategy)
    {
        if (numObjects <= 1) return 1;

        // Prepare centre / extent arrays
        std::vector<glm::vec3> centres(numObjects);
        std::vector<glm::vec3> extents(numObjects);

        for (int i = 0; i < numObjects; ++i)
        {
            Aabb aabb = registry.GetComponent<BoundingComponent>(objects[i]).GetAABB();
            if (registry.HasComponent<TransformComponent>(objects[i]))
            {
                aabb.Transform(registry.GetComponent<TransformComponent>(objects[i]).m_Model);
            }
            centres[i]  = aabb.GetCenter();
            extents[i]  = aabb.GetExtents();
        }

        int axis = 0;
        if (strategy == TDSSplitStrategy::MedianCenter || strategy == TDSSplitStrategy::KEven)
        {
            axis = Bvh::ChooseSplitAxis(centres);
        }
        else // MedianExtent
        {
            axis = Bvh::ChooseSplitAxis(extents);
        }

        // Determine split index k (median)
        int k = numObjects / 2; // default median size

        Entity* begin = objects;
        Entity* mid   = objects + k;
        Entity* end   = objects + numObjects;

        auto keyGetter = [&](Entity e){
            Aabb aabb = registry.GetComponent<BoundingComponent>(e).GetAABB();
            if (registry.HasComponent<TransformComponent>(e))
                aabb.Transform(registry.GetComponent<TransformComponent>(e).m_Model);
            return (strategy == TDSSplitStrategy::MedianExtent) ? aabb.GetExtents()[axis]
                                                               : aabb.GetCenter()[axis];
        };

        std::nth_element(begin, mid, end,
                        [&](Entity a, Entity b)
                        {
                            return keyGetter(a) < keyGetter(b);
                        });

        return k;
    }

    // BuildTopDownTree defined in global scope below
} // end anonymous namespace helpers 

// ---------------------------------------------------------------------------
// Global-scope implementation of BuildTopDownTree (pointer-based builder)
// ---------------------------------------------------------------------------

static std::unique_ptr<TreeNode> BuildTopDownTree(Registry& registry,
                                                  Registry::Entity* objects,
                                                  int numObjects,
                                                  int depth,
                                                  TDSSplitStrategy strategy,
                                                  TDSTermination termination,
                                                  TreeNode* parent)
{
    if (numObjects <= 0) return nullptr;

    constexpr int MAX_HEIGHT = 2;

    auto node = std::make_unique<TreeNode>();
    node->parent = parent;
    node->depth  = depth;

    node->aabb = ComputeAabbRange(registry, objects, numObjects);
    // Simple sphere from aabb
    node->sphere.center = node->aabb.GetCenter();
    node->sphere.radius = glm::length(node->aabb.GetExtents());

    bool stop = false;
    switch (termination)
    {
        case TDSTermination::SingleObject:  stop = (numObjects == 1); break;
        case TDSTermination::TwoObjects:    stop = (numObjects <= 2); break;
        case TDSTermination::MaxHeight2:    stop = (depth >= MAX_HEIGHT); break;
    }

    if (stop)
    {
        node->type = BvhNodeType::Leaf;
        node->objects.reserve(numObjects);
        for (int i = 0; i < numObjects; ++i)
            node->objects.push_back(objects[i]);
        return node;
    }

    int k = PartitionObjects(registry, objects, numObjects, strategy);
    if (k <= 0 || k >= numObjects)
        k = numObjects / 2;

    node->type = BvhNodeType::Internal;
    node->lChild = BuildTopDownTree(registry, objects, k, depth+1, strategy, termination, node.get());
    node->rChild = BuildTopDownTree(registry, objects + k, numObjects - k, depth+1, strategy, termination, node.get());

    return node;
} 