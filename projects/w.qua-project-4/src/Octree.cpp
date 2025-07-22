#include "Octree.hpp"
#include "Geometry.hpp"  
#include "SpatialTreeUtils.hpp"

Octree::Octree(Registry& registry, int maxObjectsPerCell, StraddlingMethod method, int maxDepth)
    : m_Registry(registry),
      m_MaxObjects(maxObjectsPerCell),
      m_Method(method),
      m_MaxDepth(maxDepth)
{
}

void Octree::GetChildIndex(const TreeNode* pNode,
                       const glm::vec3& objCenter,
                       const glm::vec3& objExtents,
                       int& outIndex,
                       bool& outStraddle)
{
    outStraddle = false;
    outIndex    = 0;

    for (int axis = 0; axis < 3; ++axis)
    {
        float d = objCenter[axis] - pNode->center[axis];

        // Check if d is within bounds of the BV
        if (std::abs(d) <= objExtents[axis])
        {
            outStraddle = true;

        }

        // which of + or - value for the bit?
        if (d > 0.0f)
            outIndex |= (1 << axis); 
    }
}

void Octree::DistributeObjectsToChildren(const TreeNode* pNode,
                                         const std::vector<Registry::Entity>& entities,
                                         std::vector<Registry::Entity> childEntities[8],
                                         std::vector<Registry::Entity>& straddlingEntities)
{
    straddlingEntities.clear();
    for (int i = 0; i < 8; ++i)
        childEntities[i].clear();

    for (auto entity : entities)
    {
        auto& t  = m_Registry.GetComponent<TransformComponent>(entity);
        auto& bc = m_Registry.GetComponent<BoundingComponent>(entity);
        Aabb worldAabb = bc.GetAABB();
        worldAabb.Transform(t.m_Model);

        glm::vec3 objCenter  = worldAabb.GetCenter();
        glm::vec3 objExtents = worldAabb.GetExtents();

        int childIdx;
        bool straddle;
        GetChildIndex(pNode, objCenter, objExtents, childIdx, straddle);

        if (straddle)
        {
            switch (m_Method)
            {
                case StraddlingMethod::UseCenter:
                    // Place in child containing center (default check)
                    childEntities[childIdx].push_back(entity);
                    break;
                    
                case StraddlingMethod::StayAtCurrentLevel:
                    // Keep at current level
                    straddlingEntities.push_back(entity);
                    break;
            }
        }
        else
        {
            // Object fits in one child
            childEntities[childIdx].push_back(entity);
        }
    }
}

std::unique_ptr<TreeNode> Octree::BuildOctree(const glm::vec3& center, float halfWidth, 
                                                      const std::vector<Registry::Entity>& entities, int level)
{
    auto node = std::make_unique<TreeNode>(center, halfWidth, level);

    bool shouldTerminate =
                           level >= m_MaxDepth ||
                           static_cast<int>(entities.size()) <= m_MaxObjects ||
                           entities.empty();

    if (shouldTerminate)
    {
        node->pObjects = entities;
        return node;
    }

    std::vector<Registry::Entity> childEntities[8];
    std::vector<Registry::Entity> straddlingEntities;
    
    DistributeObjectsToChildren(node.get(), entities, childEntities, straddlingEntities);
    
    node->pObjects = straddlingEntities;
    
    int totalChildObjects = 0;
    for (int i = 0; i < 8; ++i)
    {
        totalChildObjects += static_cast<int>(childEntities[i].size());
    }
    
    if (totalChildObjects == 0)
    {
        node->pObjects = entities;
        return node;
    }


    // Create children only if they have objects
    float childHalf = halfWidth * 0.5f;
    for (int i = 0; i < 8; ++i)
    {
        if (!childEntities[i].empty())
        {
            glm::vec3 offset(
                (i & 1) ? childHalf : -childHalf,
                (i & 2) ? childHalf : -childHalf,
                (i & 4) ? childHalf : -childHalf);
            glm::vec3 childCenter = center + offset;
            
            node->children[i] = BuildOctree(childCenter, childHalf, 
                                                   childEntities[i], level + 1);
        }
    }

    return node;
}

void Octree::Build()
{
    if (!m_Dirty) return;

    m_Root.reset();

    Aabb rootBounds(glm::vec3(0.0f), 1.0f);
    SpatialTreeUtils::ComputeSceneBounds(m_Registry, rootBounds);

    glm::vec3 center = rootBounds.GetCenter();
    float halfWidth  = rootBounds.GetExtents().x;

    std::vector<Registry::Entity> allEntities;
    auto view = m_Registry.View<TransformComponent, BoundingComponent>();
    for (auto entity : view)
    {
        allEntities.push_back(entity);
    }

    if (!allEntities.empty())
    {
        m_Root = BuildOctree(center, halfWidth, allEntities, 0);
    }

    m_Dirty = false;
}

static void GatherTreeNodes(TreeNode* node, std::vector<TreeNode*>& out)
{
    if (!node) return;
    
    out.push_back(node);
    
    for (auto& childPtr : node->children)
    {
        GatherTreeNodes(childPtr.get(), out);
    }
}

void Octree::CollectRenderables(const std::shared_ptr<Shader>& shader,
                                std::vector<std::shared_ptr<CubeRenderer>>& out)
{
    Build(); 
    out.clear();

    if (!m_Root) return;

    std::vector<TreeNode*> nodes;
    GatherTreeNodes(m_Root.get(), nodes);

    for (TreeNode* node : nodes)
    {
        glm::vec3 center = node->center;
        glm::vec3 size   = glm::vec3(node->halfwidth * 2.0f);
        
        glm::vec3 color = SpatialTreeUtils::LevelColor(node->level);

        auto cube = std::make_shared<CubeRenderer>(center, size, color, true /*wireframe*/);
        cube->Initialize(shader);
        out.push_back(std::move(cube));
    }
} 

const TreeNode* Octree::GetRoot() const
{
    return m_Root.get();
} 