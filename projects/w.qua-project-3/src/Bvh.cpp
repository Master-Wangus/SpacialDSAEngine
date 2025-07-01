#include "Bvh.hpp"
#include "CubeRenderer.hpp"
#include "SphereRenderer.hpp"
#include "Shader.hpp"

// Forward declaration
static std::unique_ptr<TreeNode> BuildTopDownTree(Registry& registry,
                                                 Registry::Entity* objects,
                                                 int numObjects,
                                                 int depth,
                                                 TDSSplitStrategy strategy,
                                                 TDSTermination termination,
                                                 TreeNode* parent = nullptr);

namespace
{
    using Entity = Registry::Entity;

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

    inline Sphere Union(const Sphere& s1, const Sphere& s2)
    {
        // If one sphere fully contains the other, return the larger.
        float centreDist = glm::distance(s1.center, s2.center);
        if (s1.radius >= centreDist + s2.radius) return s1; // s1 contains s2
        if (s2.radius >= centreDist + s1.radius) return s2; // s2 contains s1

        // Else, build the minimal enclosing sphere.
        glm::vec3 dir = glm::normalize(s2.center - s1.center);
        if (centreDist < 1e-5f) dir = glm::vec3(1,0,0); // avoid NaN when almost same centre

        float newRad = 0.5f * (centreDist + s1.radius + s2.radius);
        glm::vec3 newCenter = s1.center + dir * (newRad - s1.radius);
        return { newCenter, newRad };
    }

    // ──────────────────────────────────────────────────────────────────────
    // OBB helpers
    // ──────────────────────────────────────────────────────────────────────

    inline float Volume(const Obb& o)
    {
        glm::vec3 full = o.halfExtents * 2.0f;
        return full.x * full.y * full.z;
    }

    inline float SurfaceArea(const Obb& o)
    {
        glm::vec3 full = o.halfExtents * 2.0f;
        return 2.0f * (full.x * full.y + full.y * full.z + full.x * full.z);
    }

    inline void CollectCorners(const Obb& obb, std::vector<glm::vec3>& out)
    {
        for (int i = 0; i < 8; ++i)
        {
            glm::vec3 dir(
                (i & 1) ? 1.f : -1.f,
                (i & 2) ? 1.f : -1.f,
                (i & 4) ? 1.f : -1.f);
            glm::vec3 corner = obb.center +
                dir.x * obb.axes[0] * obb.halfExtents.x +
                dir.y * obb.axes[1] * obb.halfExtents.y +
                dir.z * obb.axes[2] * obb.halfExtents.z;
            out.push_back(corner);
        }
    }

    inline Obb Union(const Obb& a, const Obb& b)
    {
        // Use orientation of 'a' as parent orientation
        glm::vec3 axes[3] = { a.axes[0], a.axes[1], a.axes[2] };

        std::vector<glm::vec3> pts;
        pts.reserve(16);
        CollectCorners(a, pts);
        CollectCorners(b, pts);

        glm::vec3 minProj( std::numeric_limits<float>::infinity());
        glm::vec3 maxProj(-std::numeric_limits<float>::infinity());

        for (const auto& p : pts)
        {
            glm::vec3 rel = p - a.center;
            for (int k = 0; k < 3; ++k)
            {
                float proj = glm::dot(rel, axes[k]);
                minProj[k] = std::min(minProj[k], proj);
                maxProj[k] = std::max(maxProj[k], proj);
            }
        }

        glm::vec3 newHalfExt = 0.5f * (maxProj - minProj);
        glm::vec3 centreOffset = 0.5f * (maxProj + minProj);
        glm::vec3 newCentre = a.center + axes[0] * centreOffset.x + axes[1] * centreOffset.y + axes[2] * centreOffset.z;

        return Obb(newCentre, axes, newHalfExt);
    }

    // Non-member utility: compute OBB of a contiguous entity array (world-space)
    static Obb ComputeObbRange(Registry& registry, const Registry::Entity* objs, int count)
    {
        if (count <= 0) return {};

        auto getWorldObb = [&](Registry::Entity e)
        {
            Obb o = registry.GetComponent<BoundingComponent>(e).GetOBB();
            if (registry.HasComponent<TransformComponent>(e))
            {
                const auto& model = registry.GetComponent<TransformComponent>(e).m_Model;
                // Transform centre
                o.center = glm::vec3(model * glm::vec4(o.center, 1.0f));
                // Transform axes (ignore shear)
                for (int i = 0; i < 3; ++i)
                {
                    glm::vec3 axis = glm::vec3(model * glm::vec4(o.axes[i], 0.0f));
                    float len = glm::length(axis);
                    if (len > 1e-5f)
                    {
                        o.axes[i] = glm::normalize(axis);
                        o.halfExtents[i] *= len;
                    }
                }
            }
            return o;
        };

        Obb agg = getWorldObb(objs[0]);
        for (int i = 1; i < count; ++i)
            agg = Union(agg, getWorldObb(objs[i]));

        return agg;
    }

    static Obb WorldObbFromBC(Registry& registry, Registry::Entity e)
    {
        Obb o = registry.GetComponent<BoundingComponent>(e).GetOBB();
        if (registry.HasComponent<TransformComponent>(e))
        {
            const auto& model = registry.GetComponent<TransformComponent>(e).m_Model;
            o.center = glm::vec3(model * glm::vec4(o.center, 1.0f));
            for (int i = 0; i < 3; ++i)
            {
                glm::vec3 axis = glm::vec3(model * glm::vec4(o.axes[i], 0.0f));
                float len = glm::length(axis);
                if (len > 1e-6f)
                {
                    o.axes[i] = glm::normalize(axis);
                    o.halfExtents[i] *= len;
                }
            }
        }
        return o;
    }

    // Non-member utility: compute AABB of a contiguous entity array (global scope)
    static Aabb ComputeAabbRange(Registry& registry, const Registry::Entity* objs, int count)
    {
        if (count <= 0) return {};

        auto makeWorldAabb = [&](Registry::Entity e)
            {
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

    // Returns PCA bounding sphere in world-space for an entity
    static Sphere WorldSphereFromBC(Registry& registry, Registry::Entity e)
    {
        Sphere s = registry.GetComponent<BoundingComponent>(e).GetPCASphere();
        if (registry.HasComponent<TransformComponent>(e))
        {
            const auto& model = registry.GetComponent<TransformComponent>(e).m_Model;
            s.center = glm::vec3(model * glm::vec4(s.center, 1.0f));
            float maxScale = glm::compMax(glm::vec3(glm::length(model[0]), glm::length(model[1]), glm::length(model[2])));
            s.radius *= maxScale;
        }
        return s;
    }

    // Decides how a node in the Top-down traversal is split into a left and right child
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
            centres[i] = aabb.GetCenter();
            extents[i] = aabb.GetExtents();
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
        Entity* mid = objects + k;
        Entity* end = objects + numObjects;

        auto keyGetter = [&](Entity e) {
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
}

// Static configuration defaults
BvhBuildMethod   BvhBuildConfig::s_Method        = BvhBuildMethod::TopDown;
TDSSplitStrategy BvhBuildConfig::s_TDStrategy    = TDSSplitStrategy::MedianCenter;
TDSTermination   BvhBuildConfig::s_TDTermination = TDSTermination::SingleObject;
BUSHeuristic     BvhBuildConfig::s_BUHeuristic   = BUSHeuristic::MinCombinedVolume;
BvhVolumeType    BvhBuildConfig::s_BVType = BvhVolumeType::Aabb;


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

    // Populate entity → leaf map & flat depth list for rendering
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
        if (BvhBuildConfig::s_BVType == BvhVolumeType::Aabb)
        {
            leaf->aabb   = ComputeAabbRange(registry, &entity, 1);
        }
        else if (BvhBuildConfig::s_BVType == BvhVolumeType::Sphere)
        {
            leaf->sphere = WorldSphereFromBC(registry, entity);
        }
        else // Obb
        {
            leaf->obb = WorldObbFromBC(registry, entity);
        }
        leaf->depth  = 0;

        m_EntityToLeaf[entity] = leaf.get();

        active.push_back(std::move(leaf));
    }

    auto pairCost = [&](const TreeNode* a, const TreeNode* b)
    {
        if (BvhBuildConfig::s_BVType == BvhVolumeType::Aabb)
        {
            Aabb u = Union(a->aabb, b->aabb);
            glm::vec3 ext = u.max - u.min;
            switch (heuristic)
            {
                case BUSHeuristic::NearestCenter:
                    return glm::distance(a->aabb.GetCenter(), b->aabb.GetCenter());
                case BUSHeuristic::MinCombinedVolume:
                    return ext.x * ext.y * ext.z;
                case BUSHeuristic::MinCombinedSurfaceArea:
                default:
                    return 2.f*(ext.x*ext.y + ext.y*ext.z + ext.x*ext.z);
            }
        }
        else if (BvhBuildConfig::s_BVType == BvhVolumeType::Sphere)
        {
            Sphere u = Union(a->sphere, b->sphere);
            switch (heuristic)
            {
                case BUSHeuristic::NearestCenter:
                    return glm::distance(a->sphere.center, b->sphere.center);
                case BUSHeuristic::MinCombinedVolume:
                    return (4.18879020479f) * u.radius * u.radius * u.radius; // 4/3*pi*r^3
                case BUSHeuristic::MinCombinedSurfaceArea:
                default:
                    return (12.5663706144f) * u.radius * u.radius; // 4*pi*r^2
            }
        }
        else // Obb
        {
            Obb u = Union(a->obb, b->obb);
            switch (heuristic)
            {
                case BUSHeuristic::NearestCenter:
                    return glm::distance(a->obb.center, b->obb.center);
                case BUSHeuristic::MinCombinedVolume:
                    return Volume(u);
                case BUSHeuristic::MinCombinedSurfaceArea:
                default:
                    return SurfaceArea(u);
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
        if (BvhBuildConfig::s_BVType == BvhVolumeType::Aabb)
        {
            parent->aabb  = Union(left->aabb, right->aabb);
        }
        else if (BvhBuildConfig::s_BVType == BvhVolumeType::Sphere)
        {
            parent->sphere = Union(left->sphere, right->sphere);
        }
        else
        {
            parent->obb = Union(left->obb, right->obb);
        }

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
    if (BvhBuildConfig::s_BVType == BvhVolumeType::Aabb)
    {
        if (!leaf->objects.empty())
            leaf->aabb = ComputeAabbRange(registry, leaf->objects.data(), static_cast<int>(leaf->objects.size()));
    }
    else if (BvhBuildConfig::s_BVType == BvhVolumeType::Sphere)
    {
        Sphere agg = WorldSphereFromBC(registry, leaf->objects[0]);
        for (size_t i=1;i<leaf->objects.size();++i)
            agg = Union(agg, WorldSphereFromBC(registry, leaf->objects[i]));
        leaf->sphere = agg;
    }
    else // Obb
    {
        if (!leaf->objects.empty())
        {
            Obb agg = WorldObbFromBC(registry, leaf->objects[0]);
            for (size_t i = 1; i < leaf->objects.size(); ++i)
                agg = Union(agg, WorldObbFromBC(registry, leaf->objects[i]));
            leaf->obb = agg;
        }
    }

    // Walk up the tree and update internal nodes
    for (TreeNode* node = leaf->parent; node; node = node->parent)
    {
        if (BvhBuildConfig::s_BVType == BvhVolumeType::Aabb)
        {
            if (node->lChild && node->rChild)
                node->aabb = Union(node->lChild->aabb, node->rChild->aabb);
            else if (node->lChild)
                node->aabb = node->lChild->aabb;
            else if (node->rChild)
                node->aabb = node->rChild->aabb;
        }
        else if (BvhBuildConfig::s_BVType == BvhVolumeType::Sphere)
        {
            if (node->lChild && node->rChild)
                node->sphere = Union(node->lChild->sphere, node->rChild->sphere);
            else if (node->lChild)
                node->sphere = node->lChild->sphere;
            else if (node->rChild)
                node->sphere = node->rChild->sphere;
        }
        else
        {
            if (node->lChild && node->rChild)
                node->obb = Union(node->lChild->obb, node->rChild->obb);
            else if (node->lChild)
                node->obb = node->lChild->obb;
            else if (node->rChild)
                node->obb = node->rChild->obb;
        }
    }
}

Aabb Bvh::ComputeAabb(Registry& registry, const std::vector<Entity>& objs)
{
    if (objs.empty()) return {};

    auto makeWorldAabb = [&](Entity e)
        {
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
Bvh::CreateRenderables(const std::shared_ptr<Shader>& shader, BvhVolumeType volumeType) const
{
    std::vector<std::shared_ptr<IRenderable>> result;
    m_FlatDepths.clear();

    if (!shader || !m_Root) return result;

    // Traverse pointer-based tree
    CollectRenderables(m_Root.get(), volumeType, shader, result);
    return result;
}

// Helper to recursively create renderables from pointer-based tree
void Bvh::CollectRenderables(const TreeNode* node,
                             BvhVolumeType volumeType,
                             const std::shared_ptr<Shader>& shader,
                             std::vector<std::shared_ptr<IRenderable>>& out) const
{
    if (!node) return;

    static const glm::vec3 palette[7] = 
    {
        {1.0f, 0.0f, 0.0f}, // red
        {1.0f, 0.5f, 0.0f}, // orange
        {1.0f, 1.0f, 0.0f}, // yellow
        {0.0f, 1.0f, 0.0f}, // green
        {0.0f, 0.0f, 1.0f}, // blue
        {0.3f, 0.0f, 0.5f}, // dark violet
        {0.6f, 0.0f, 1.0f}  // bright violet
    };

    glm::vec3 colour = palette[node->depth % 7];

    if (volumeType == BvhVolumeType::Aabb)
    {
        glm::vec3 size = node->aabb.max - node->aabb.min;
        glm::vec3 centre = node->aabb.GetCenter();
        auto cube = std::make_shared<CubeRenderer>(centre, size, colour, true);
        cube->Initialize(shader);
        out.push_back(cube);
    }
    else if (volumeType == BvhVolumeType::Sphere)
    {
        auto sphere = std::make_shared<SphereRenderer>(node->sphere.center, node->sphere.radius, colour, true);
        sphere->Initialize(shader);
        out.push_back(sphere);
    }
    else
    {
        auto obbCube = std::make_shared<CubeRenderer>(node->obb.center, node->obb.axes, node->obb.halfExtents, colour, true);
        obbCube->Initialize(shader);
        out.push_back(obbCube);
    }

    // record depth parallel to out
    m_FlatDepths.push_back(node->depth);

    CollectRenderables(node->lChild.get(), volumeType, shader, out);
    CollectRenderables(node->rChild.get(), volumeType, shader, out);
}

const std::vector<int>& Bvh::GetDepths() const
{
    return m_FlatDepths;
}

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

    if (BvhBuildConfig::s_BVType == BvhVolumeType::Aabb)
    {
        node->aabb = ComputeAabbRange(registry, objects, numObjects);
    }
    else if (BvhBuildConfig::s_BVType == BvhVolumeType::Sphere)
    {
        // Aggregate world-space spheres
        Sphere agg = WorldSphereFromBC(registry, objects[0]);
        for (int i=1;i<numObjects;++i)
            agg = Union(agg, WorldSphereFromBC(registry, objects[i]));
        node->sphere = agg;
    }
    else // Obb
    {
        node->obb = ComputeObbRange(registry, objects, numObjects);
    }

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

    // Update parent bounds from children
    if (BvhBuildConfig::s_BVType == BvhVolumeType::Aabb)
    {
        node->aabb   = Union(node->lChild->aabb, node->rChild->aabb);
    }
    else if (BvhBuildConfig::s_BVType == BvhVolumeType::Sphere)
    {
        node->sphere = Union(node->lChild->sphere, node->rChild->sphere);
    }
    else
    {
        node->obb = Union(node->lChild->obb, node->rChild->obb);
    }

    return node;
} 