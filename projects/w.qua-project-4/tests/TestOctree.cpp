#include <gtest/gtest.h>
#include "Octree.hpp"
#include "Registry.hpp"
#include "Components.hpp"
#include "Shapes.hpp"

class OctreeTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        registry = std::make_unique<Registry>();
        // Use 4 objects per cell to force predictable subdivision
        octree = std::make_unique<Octree>(*registry, 4, StraddlingMethod::UseCenter, 5);
    }

    void TearDown() override
    {
        octree.reset();
        registry.reset();
    }

    // Helper to create a simple entity with transform and bounding box centred at 'position'
    Registry::Entity CreateTestEntity(const glm::vec3& position,
                                      const glm::vec3& scale = glm::vec3(0.1f))
    {
        auto entity = registry->Create();
        auto& transform = registry->AddComponent<TransformComponent>(entity,
                                                                    position,
                                                                    glm::vec3(0.0f),
                                                                    scale);
        auto& bounds = registry->AddComponent<BoundingComponent>(entity);
        bounds.m_AABB = Aabb(position - scale * 0.5f, position + scale * 0.5f);
        bounds.m_AABBComputed = true;
        return entity;
    }

    std::unique_ptr<Registry> registry;
    std::unique_ptr<Octree>   octree;
};

// Basic construction test
TEST_F(OctreeTest, Construction)
{
    ASSERT_NE(octree.get(), nullptr);
}

// Building with no entities should not throw
TEST_F(OctreeTest, BuildEmpty)
{
    EXPECT_NO_THROW(octree->Build());
}

// Building with a single entity should succeed
TEST_F(OctreeTest, BuildSingleEntity)
{
    CreateTestEntity(glm::vec3(0.0f));
    EXPECT_NO_THROW(octree->Build());
}

// Stress test – 32 objects, 4 per octant
TEST_F(OctreeTest, Stress32Objects)
{
    octree->SetMaxObjectsPerCell(4);

    // Coordinates to address each octant (neg/pos along axes)
    const float sign[2] = { -0.25f, 0.25f };
    int created = 0;

    // Create 4 tightly-packed objects per octant
    for (int xi = 0; xi < 2; ++xi)
        for (int yi = 0; yi < 2; ++yi)
            for (int zi = 0; zi < 2; ++zi)
            {
                glm::vec3 base(sign[xi], sign[yi], sign[zi]);
                for (int i = 0; i < 4; ++i)
                {
                    glm::vec3 jitter( (i & 1) ? 0.02f : -0.02f,
                                       (i & 2) ? 0.02f : -0.02f,
                                       0.0f );
                    CreateTestEntity(base + jitter);
                    ++created;
                }
            }

    ASSERT_EQ(created, 32);

    octree->Build();

    const TreeNode* root = octree->GetRoot();
    ASSERT_NE(root, nullptr);

    // Verify that each of the 8 children exists and holds exactly 4 objects
    int totalObjects = 0;
    for (int i = 0; i < 8; ++i)
    {
        const TreeNode* child = root->children[i].get();
        ASSERT_NE(child, nullptr) << "Child " << i << " should exist";
        EXPECT_EQ(child->pObjects.size(), 4u) << "Child " << i << " should contain 4 objects";
        totalObjects += static_cast<int>(child->pObjects.size());
    }

    EXPECT_EQ(totalObjects, 32);
} 