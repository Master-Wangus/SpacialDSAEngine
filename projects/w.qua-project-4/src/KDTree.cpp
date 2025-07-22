#include "KDTree.hpp"
#include "Geometry.hpp"
#include "SpatialTreeUtils.hpp"

KDTree::KDTree(Registry& registry, int maxObjectsPerNode, KdSplitMethod splitMethod, int maxDepth)
    : m_Registry(registry),
      m_MaxObjects(maxObjectsPerNode),
      m_SplitMethod(splitMethod),
      m_MaxDepth(maxDepth)
{
}

float KDTree::ChooseSplitPosition(const std::vector<Registry::Entity>& entities, int axis)
{
    std::vector<float> values;
    values.reserve(entities.size());

    for (auto entity : entities)
    {
        auto& t  = m_Registry.GetComponent<TransformComponent>(entity);
        auto& bc = m_Registry.GetComponent<BoundingComponent>(entity);
        Aabb box = bc.GetAABB();
        box.Transform(t.m_Model);

        if (m_SplitMethod == KdSplitMethod::MedianCenter)
        {
            values.push_back(box.GetCenter()[axis]);
        }
        else // MedianExtent 
        {
            values.push_back(box.GetExtents()[axis]);
        }
    }

    if (values.empty()) return 0.0f;

    // Wesley: nth_element sorts all values based on the middle index, left values < middle index < right values.
    //         We return the middle index as the partition
    std::nth_element(values.begin(), values.begin() + values.size() / 2, values.end());
    return values[values.size() / 2];
}

std::unique_ptr<KdNode> KDTree::BuildKdTree(const std::vector<Registry::Entity>& entities,
                                               const Aabb& bounds,
                                               int level)
{
    auto node = std::make_unique<KdNode>(bounds, level);

    if (entities.empty() || level >= m_MaxDepth || static_cast<int>(entities.size()) <= m_MaxObjects)
    {
        node->objects = entities;
        return node;
    }

    int axis = level % 3; // X, Y, Z cycling
    float splitPos = ChooseSplitPosition(entities, axis);

    node->axis  = axis;
    node->split = splitPos;

    std::vector<Registry::Entity> leftSet;
    std::vector<Registry::Entity> rightSet;

    leftSet.reserve(entities.size());
    rightSet.reserve(entities.size());

    for (auto entity : entities)
    {
        auto& t  = m_Registry.GetComponent<TransformComponent>(entity);
        auto& bc = m_Registry.GetComponent<BoundingComponent>(entity);
        Aabb box = bc.GetAABB();
        box.Transform(t.m_Model);

        float centerVal = box.GetCenter()[axis];
        if (centerVal < splitPos)
        {
            leftSet.push_back(entity);
        }
        else
        {
            rightSet.push_back(entity);
        }
    }

    // If one side empty -> terminate
    if (leftSet.empty() || rightSet.empty())
    {
        node->objects = entities;
        return node;
    }

    // Create child bounds by splitting parent bounds along axis
    glm::vec3 minLeft  = bounds.min;
    glm::vec3 maxLeft  = bounds.max;
    glm::vec3 minRight = bounds.min;
    glm::vec3 maxRight = bounds.max;

    maxLeft[axis]  = splitPos;
    minRight[axis] = splitPos;

    node->left  = BuildKdTree(leftSet,  Aabb(minLeft,  maxLeft),  level + 1);
    node->right = BuildKdTree(rightSet, Aabb(minRight, maxRight), level + 1);

    return node;
}

void KDTree::Build()
{
    if (!m_Dirty) return;

    m_Root.reset();

    std::vector<Registry::Entity> allEntities;
    for (auto entity : m_Registry.View<TransformComponent, BoundingComponent>())
    {
        allEntities.push_back(entity);
    }

    if (allEntities.empty())
    {
        m_Dirty = false;
        return;
    }

    Aabb sceneBounds;
    SpatialTreeUtils::ComputeSceneBounds(m_Registry, sceneBounds);

    m_Root = BuildKdTree(allEntities, sceneBounds, 0);

    m_Dirty = false;
}

static void GatherKdNodes(KdNode* node, std::vector<KdNode*>& out)
{
    if (!node) return;
    out.push_back(node);
    GatherKdNodes(node->left.get(),  out);
    GatherKdNodes(node->right.get(), out);
}

void KDTree::CollectRenderables(const std::shared_ptr<Shader>& shader,
                                std::vector<std::shared_ptr<CubeRenderer>>& out)
{
    Build();
    out.clear();

    if (!m_Root) return;

    std::vector<KdNode*> nodes;
    GatherKdNodes(m_Root.get(), nodes);

    for (KdNode* node : nodes)
    {
        glm::vec3 size    = node->bounds.GetExtents() * 2.0f;
        glm::vec3 center  = node->bounds.GetCenter();
        glm::vec3 color = SpatialTreeUtils::LevelColor(node->level);

        auto cube = std::make_shared<CubeRenderer>(center, size, color, true /*wireframe*/);
        cube->Initialize(shader);
        out.push_back(std::move(cube));
    }
} 