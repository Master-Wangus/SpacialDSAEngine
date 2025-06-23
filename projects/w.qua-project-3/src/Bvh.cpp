#include "Bvh.hpp"
#include <queue>
#include <numeric>
#include "CubeRenderer.hpp"
#include "SphereRenderer.hpp"
#include "Shader.hpp"

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
}

// ──────────────────────────────────────────────────────────────────────────────
// Public interface
// ──────────────────────────────────────────────────────────────────────────────

void Bvh::Clear()
{
    m_Nodes.clear();
    m_RootIndex = -1;
    m_EntityToLeaf.clear();
}

void Bvh::BuildTopDown(Registry& registry,
                       const std::vector<Entity>& objects,
                       TDSSplitStrategy strategy,
                       TDSTermination termination,
                       size_t maxHeight)
{
    Clear();
    if (objects.empty()) return;

    // Copy so we can sort/partition freely
    std::vector<Entity> objs = objects;
    m_RootIndex = BuildTopDownRecursive(registry, objs, 0, strategy, termination, maxHeight);
}

void Bvh::BuildBottomUp(Registry& registry,
                        const std::vector<Entity>& objects,
                        BUSHeuristic heuristic)
{
    Clear();
    if (objects.empty()) return;

    // Start with a node per object (leaves)
    for (auto entity : objects)
    {
        BvhNode leaf;
        leaf.m_Objects.push_back(entity);
        leaf.m_AABB = ComputeAabb(registry, leaf.m_Objects);
        leaf.m_Sphere = ComputeSphereFromAabb(leaf.m_AABB);
        leaf.m_Depth = 0;
        m_Nodes.push_back(std::move(leaf));
        m_EntityToLeaf[entity] = static_cast<int>(m_Nodes.size())-1;
    }

    // Active list holds indices of current forest roots
    std::vector<int> active(m_Nodes.size());
    std::iota(active.begin(), active.end(), 0);

    // Greedy bottom-up merging
    while (active.size() > 1)
    {
        float bestCost = std::numeric_limits<float>::max();
        size_t bestI = 0, bestJ = 1;

        // Naive O(n^2) search for best pair
        for (size_t i = 0; i < active.size(); ++i)
        {
            for (size_t j = i + 1; j < active.size(); ++j)
            {
                float c = PairCost(m_Nodes[active[i]], m_Nodes[active[j]], heuristic);
                if (c < bestCost)
                {
                    bestCost = c;
                    bestI = i;
                    bestJ = j;
                }
            }
        }

        // Merge the best pair
        int leftIdx  = active[bestI];
        int rightIdx = active[bestJ];

        BvhNode parent;
        parent.m_Left  = leftIdx;
        parent.m_Right = rightIdx;
        parent.m_Depth = std::max(m_Nodes[leftIdx].m_Depth, m_Nodes[rightIdx].m_Depth) + 1;
        parent.m_AABB  = Union(m_Nodes[leftIdx].m_AABB, m_Nodes[rightIdx].m_AABB);
        parent.m_Sphere = ComputeSphereFromAabb(parent.m_AABB);

        int parentIdx = static_cast<int>(m_Nodes.size());
        m_Nodes.push_back(std::move(parent));

        m_Nodes[leftIdx].m_Parent  = parentIdx;
        m_Nodes[rightIdx].m_Parent = parentIdx;

        // Replace active pair with parent
        if (bestI > bestJ) std::swap(bestI, bestJ);
        active.erase(active.begin() + bestJ);
        active.erase(active.begin() + bestI);
        active.push_back(parentIdx);
    }

    m_RootIndex = active.front();
}

void Bvh::RefitLeaf(Registry& registry, Entity entity)
{
    auto it = m_EntityToLeaf.find(entity);
    if (it == m_EntityToLeaf.end()) return;

    int idx = it->second;
    if (idx < 0 || idx >= static_cast<int>(m_Nodes.size())) return;

    // Update leaf BV
    BvhNode& leaf = m_Nodes[idx];
    leaf.m_AABB = ComputeAabb(registry, leaf.m_Objects);
    leaf.m_Sphere = ComputeSphereFromAabb(leaf.m_AABB);

    // Walk up and refit internal nodes
    int parentIdx = leaf.m_Parent;
    while (parentIdx != -1)
    {
        BvhNode& parent = m_Nodes[parentIdx];
        const BvhNode& left  = m_Nodes[parent.m_Left];
        const BvhNode& right = m_Nodes[parent.m_Right];
        parent.m_AABB = Union(left.m_AABB, right.m_AABB);
        parent.m_Sphere = ComputeSphereFromAabb(parent.m_AABB);

        parentIdx = parent.m_Parent;
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

int Bvh::BuildTopDownRecursive(Registry& registry,
                               std::vector<Entity>& objects,
                               int depth,
                               TDSSplitStrategy strategy,
                               TDSTermination termination,
                               size_t maxHeight)
{
    // Termination checks ------------------------------------------------------
    if (objects.empty()) return -1;
    if ((termination == TDSTermination::SingleObject  && objects.size() == 1) ||
        (termination == TDSTermination::TwoObjects    && objects.size() <= 2) ||
        (termination == TDSTermination::MaxHeight2    && depth >= static_cast<int>(maxHeight)))
    {
        // Create leaf node
        BvhNode leaf;
        leaf.m_Depth = depth;
        leaf.m_Objects = objects;
        leaf.m_AABB = ComputeAabb(registry, objects);
        leaf.m_Sphere = ComputeSphereFromAabb(leaf.m_AABB);

        int idx = static_cast<int>(m_Nodes.size());
        m_Nodes.push_back(std::move(leaf));
        // Map each object to this leaf for quick updates
        for (auto e : objects)
        {
            m_EntityToLeaf[e] = idx;
        }
        return idx;
    }

    // Choose split axis based on extents or centres ----------------------------------
    std::vector<float> keys(objects.size());
    std::vector<glm::vec3> extentVec(objects.size());

    // Fill keys/extentVec
    for (size_t i = 0; i < objects.size(); ++i)
    {
        auto& bc = registry.GetComponent<BoundingComponent>(objects[i]);
        Aabb aabb = bc.GetAABB();
        if (registry.HasComponent<TransformComponent>(objects[i]))
        {
            aabb.Transform(registry.GetComponent<TransformComponent>(objects[i]).m_Model);
        }
        extentVec[i] = aabb.GetExtents();
    }

    int axis = 0;
    if (strategy == TDSSplitStrategy::MedianCenter || strategy == TDSSplitStrategy::KEven)
    {
        // Use spread of centres
        std::vector<glm::vec3> centres(objects.size());
        for (size_t i = 0; i < objects.size(); ++i)
        {
            Aabb aabb = registry.GetComponent<BoundingComponent>(objects[i]).GetAABB();
            if (registry.HasComponent<TransformComponent>(objects[i]))
            {
                aabb.Transform(registry.GetComponent<TransformComponent>(objects[i]).m_Model);
            }
            centres[i] = aabb.GetCenter();
        }
        axis = ChooseSplitAxis(centres);
        for (size_t i = 0; i < objects.size(); ++i) keys[i] = centres[i][axis];
    }
    else if (strategy == TDSSplitStrategy::MedianExtent)
    {
        axis = ChooseSplitAxis(extentVec);
        for (size_t i = 0; i < objects.size(); ++i) keys[i] = extentVec[i][axis];
    }

    // Partition around median
    size_t midIndex = objects.size() / 2;
    std::nth_element(objects.begin(), objects.begin() + midIndex, objects.end(),
                     [&](Entity a, Entity b)
                     {
                         Aabb aA = registry.GetComponent<BoundingComponent>(a).GetAABB();
                         Aabb bA = registry.GetComponent<BoundingComponent>(b).GetAABB();
                         if (registry.HasComponent<TransformComponent>(a))
                             aA.Transform(registry.GetComponent<TransformComponent>(a).m_Model);
                         if (registry.HasComponent<TransformComponent>(b))
                             bA.Transform(registry.GetComponent<TransformComponent>(b).m_Model);
                         return aA.GetCenter()[axis] < bA.GetCenter()[axis];
                     });

    std::vector<Entity> leftObjs(objects.begin(), objects.begin() + midIndex);
    std::vector<Entity> rightObjs(objects.begin() + midIndex, objects.end());

    // Create internal node first (placeholder children indices)
    int nodeIdx = static_cast<int>(m_Nodes.size());
    m_Nodes.emplace_back(); // default constructed
    m_Nodes[nodeIdx].m_Depth = depth;

    // Recurse
    int leftIdx  = BuildTopDownRecursive(registry, leftObjs,  depth + 1, strategy, termination, maxHeight);
    int rightIdx = BuildTopDownRecursive(registry, rightObjs, depth + 1, strategy, termination, maxHeight);

    m_Nodes[nodeIdx].m_Left  = leftIdx;
    m_Nodes[nodeIdx].m_Right = rightIdx;
    if (leftIdx != -1)  m_Nodes[leftIdx].m_Parent  = nodeIdx;
    if (rightIdx != -1) m_Nodes[rightIdx].m_Parent = nodeIdx;

    // Compute bounds from children
    if (leftIdx != -1 && rightIdx != -1)
    {
        m_Nodes[nodeIdx].m_AABB = Union(m_Nodes[leftIdx].m_AABB, m_Nodes[rightIdx].m_AABB);
    }
    else if (leftIdx != -1)
    {
        m_Nodes[nodeIdx].m_AABB = m_Nodes[leftIdx].m_AABB;
    }
    else if (rightIdx != -1)
    {
        m_Nodes[nodeIdx].m_AABB = m_Nodes[rightIdx].m_AABB;
    }
    m_Nodes[nodeIdx].m_Sphere = ComputeSphereFromAabb(m_Nodes[nodeIdx].m_AABB);

    return nodeIdx;
}

float Bvh::PairCost(const BvhNode& a, const BvhNode& b, BUSHeuristic h) const
{
    Aabb ab = Union(a.m_AABB, b.m_AABB);

    switch (h)
    {
        case BUSHeuristic::NearestCenter:
        {
            float dist = glm::distance(a.m_AABB.GetCenter(), b.m_AABB.GetCenter());
            return dist;
        }
        case BUSHeuristic::MinCombinedVolume:
            return Volume(ab);
        case BUSHeuristic::MinCombinedSurfaceArea:
            return SurfaceArea(ab);
        default:
            return Volume(ab);
    }
}

std::vector<std::shared_ptr<IRenderable>>
Bvh::CreateRenderables(const std::shared_ptr<Shader>& shader, bool useAabb) const
{
    std::vector<std::shared_ptr<IRenderable>> result;
    if (!shader) return result;

    // Colour palette for first 7 levels (ROYGBIV)
    static const glm::vec3 palette[7] = {
        {1.0f, 0.0f, 0.0f}, // Red
        {1.0f, 0.5f, 0.0f}, // Orange
        {1.0f, 1.0f, 0.0f}, // Yellow
        {0.0f, 1.0f, 0.0f}, // Green
        {0.0f, 0.0f, 1.0f}, // Blue
        {0.3f, 0.0f, 0.5f}, // Indigo
        {0.6f, 0.0f, 1.0f}  // Violet
    };

    for (const auto& node : m_Nodes)
    {
        glm::vec3 colour = palette[node.m_Depth % 7];
        if (useAabb)
        {
            glm::vec3 size = node.m_AABB.max - node.m_AABB.min;
            glm::vec3 centre = node.m_AABB.GetCenter();
            auto cube = std::make_shared<CubeRenderer>(centre, size, colour, true);
            cube->Initialize(shader);
            result.push_back(cube);
        }
        else
        {
            auto sphere = std::make_shared<SphereRenderer>(node.m_Sphere.center, node.m_Sphere.radius, colour, true);
            sphere->Initialize(shader);
            result.push_back(sphere);
        }
    }
    return result;
} 