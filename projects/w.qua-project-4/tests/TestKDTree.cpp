#include <gtest/gtest.h>
#include <algorithm>
#include "KDTree.hpp"
#include "Registry.hpp"
#include "Components.hpp"
#include "Shapes.hpp"

class KDTreeTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        registry = std::make_unique<Registry>();
        // 4 objects per node to guarantee subdivision with 32 objects
        kdtree = std::make_unique<KDTree>(*registry, 4, KdSplitMethod::MedianCenter, 10);
    }

    void TearDown() override
    {
        kdtree.reset();
        registry.reset();
    }

    Registry::Entity CreateTestEntity(const glm::vec3& position,
                                      const glm::vec3& scale = glm::vec3(0.1f))
    {
        auto entity = registry->Create();
        registry->AddComponent<TransformComponent>(entity, position, glm::vec3(0.0f), scale);
        auto& bounds = registry->AddComponent<BoundingComponent>(entity);
        bounds.m_AABB = Aabb(position - scale * 0.5f, position + scale * 0.5f);
        bounds.m_AABBComputed = true;
        return entity;
    }

    std::unique_ptr<Registry> registry;
    std::unique_ptr<KDTree>   kdtree;
};

TEST_F(KDTreeTest, Construction)
{
    ASSERT_NE(kdtree.get(), nullptr);
}

TEST_F(KDTreeTest, BuildEmpty)
{
    EXPECT_NO_THROW(kdtree->Build());
}

TEST_F(KDTreeTest, BuildSingleEntity)
{
    CreateTestEntity(glm::vec3(0.0f));
    EXPECT_NO_THROW(kdtree->Build());
}

TEST_F(KDTreeTest, SplitMethods)
{
    CreateTestEntity(glm::vec3(-0.5f));
    CreateTestEntity(glm::vec3(0.5f));

    kdtree->SetSplitMethod(KdSplitMethod::MedianCenter);
    EXPECT_NO_THROW(kdtree->Build());

    kdtree->SetSplitMethod(KdSplitMethod::MedianExtent);
    EXPECT_NO_THROW(kdtree->Build());
}

// Helper: collect leaf nodes recursively
static void CollectLeaves(const KdNode* node, std::vector<const KdNode*>& leaves)
{
    if (!node) return;

    if (!node->left && !node->right)
    {
        leaves.push_back(node);
        return;
    }
    CollectLeaves(node->left.get(),  leaves);
    CollectLeaves(node->right.get(), leaves);
}

TEST_F(KDTreeTest, Stress32Objects)
{
    kdtree->SetMaxObjectsPerNode(4);

    // Create a predictable 4 x 4 x 2 grid = 32 objects inside roughly unit cube
    const float xs[4] = { -0.75f, -0.25f, 0.25f, 0.75f };
    const float ys[4] = { -0.75f, -0.25f, 0.25f, 0.75f };
    const float zs[2] = { -0.25f, 0.25f };

    int created = 0;
    for (float x : xs)
        for (float y : ys)
            for (float z : zs)
            {
                CreateTestEntity(glm::vec3(x, y, z));
                ++created;
            }

    ASSERT_EQ(created, 32);

    kdtree->Build();
    const KdNode* root = kdtree->GetRoot();
    ASSERT_NE(root, nullptr);

    // Gather leaves and validate each contains at most 4 objects
    std::vector<const KdNode*> leaves;
    CollectLeaves(root, leaves);

    size_t totalObjects = 0;
    for (const KdNode* leaf : leaves)
    {
        EXPECT_LE(leaf->objects.size(), 4u);
        totalObjects += leaf->objects.size();
    }

    EXPECT_EQ(totalObjects, 32u);
} 