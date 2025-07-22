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
        octree = std::make_unique<Octree>(*registry, 5, StraddlingMethod::UseCenter, 4);
    }
    
    void TearDown() override 
    {
        octree.reset();
        registry.reset();
    }
    
    // Helper function to create a simple entity with transform and bounds
    Registry::Entity CreateTestEntity(const glm::vec3& position, const glm::vec3& scale = glm::vec3(1.0f)) 
    {
        auto entity = registry->Create();
        
        auto& transform = registry->AddComponent<TransformComponent>(entity, position, glm::vec3(0.0f), scale);        
        auto& bounds = registry->AddComponent<BoundingComponent>(entity);
        bounds.m_AABB = Aabb(position - scale * 0.5f, position + scale * 0.5f);
        bounds.m_AABBComputed = true;
        
        return entity;
    }

    std::unique_ptr<Registry> registry;
    std::unique_ptr<Octree> octree;
};

// Test TreeNode construction
TEST_F(OctreeTest, TreeNodeConstruction) 
{
    glm::vec3 center(0.0f, 0.0f, 0.0f);
    float halfwidth = 10.0f;
    int level = 0;
    
    TreeNode node(center, halfwidth, level);
    
    EXPECT_EQ(node.center, center);
    EXPECT_EQ(node.halfwidth, halfwidth);
    EXPECT_EQ(node.level, level);
    EXPECT_TRUE(node.pObjects.empty());
    
    for (int i = 0; i < 8; ++i) 
    {
        EXPECT_EQ(node.pChildren[i], nullptr);
    }
}

// Test Octree construction
TEST_F(OctreeTest, OctreeConstruction) 
{
    EXPECT_NE(octree.get(), nullptr);
    
    octree->SetMaxObjectsPerCell(10);
    octree->SetStraddlingMethod(StraddlingMethod::UseCenter);
}

// Test building empty octree
TEST_F(OctreeTest, BuildEmptyOctree) 
{
    EXPECT_NO_THROW(octree->Build());
}

// Test building octree with single entity
TEST_F(OctreeTest, BuildSingleEntity) 
{
    CreateTestEntity(glm::vec3(0.0f, 0.0f, 0.0f));
    
    EXPECT_NO_THROW(octree->Build());
}

// Test building octree with multiple entities
TEST_F(OctreeTest, BuildMultipleEntities) 
{
    CreateTestEntity(glm::vec3(-5.0f, -5.0f, -5.0f));
    CreateTestEntity(glm::vec3(5.0f, 5.0f, 5.0f));
    CreateTestEntity(glm::vec3(-5.0f, 5.0f, -5.0f));
    CreateTestEntity(glm::vec3(5.0f, -5.0f, 5.0f));
    
    EXPECT_NO_THROW(octree->Build());
}

// Test building octree with many entities (should trigger subdivision)
TEST_F(OctreeTest, BuildManyEntities) 
{
    // Create more entities than maxObjectsPerCell to trigger subdivision
    for (int i = 0; i < 10; ++i)
    {
        for (int j = 0; j < 10; ++j) 
        {
            CreateTestEntity(glm::vec3(i * 2.0f - 10.0f, j * 2.0f - 10.0f, 0.0f), glm::vec3(0.5f));
        }
    }
    
    EXPECT_NO_THROW(octree->Build());
}

// Test different straddling methods
TEST_F(OctreeTest, StraddlingMethods) 
{
    CreateTestEntity(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(2.0f));
    CreateTestEntity(glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(1.0f));
    
    // Test UseCenter method
    octree->SetStraddlingMethod(StraddlingMethod::UseCenter);
    EXPECT_NO_THROW(octree->Build());
    
    // Test StayAtCurrentLevel method
    octree->SetStraddlingMethod(StraddlingMethod::StayAtCurrentLevel);
    EXPECT_NO_THROW(octree->Build());
}

// Test parameter changes
TEST_F(OctreeTest, ParameterChanges) 
{
    // Create some entities
    CreateTestEntity(glm::vec3(0.0f, 0.0f, 0.0f));
    CreateTestEntity(glm::vec3(1.0f, 1.0f, 1.0f));
    
    octree->Build();
    
    // Change max objects per cell
    octree->SetMaxObjectsPerCell(1);
    EXPECT_NO_THROW(octree->Build());
    
    // Change straddling method
    octree->SetStraddlingMethod(StraddlingMethod::StayAtCurrentLevel);
    EXPECT_NO_THROW(octree->Build());
}

// Test MarkDirty functionality
TEST_F(OctreeTest, MarkDirty) 
{
    // Create entities and build
    CreateTestEntity(glm::vec3(0.0f, 0.0f, 0.0f));
    octree->Build();
    
    octree->MarkDirty();
    EXPECT_NO_THROW(octree->Build());
}

// Test edge cases
TEST_F(OctreeTest, EdgeCases) 
{
    // Test with max objects per cell = 1 (should create more subdivisions)
    octree->SetMaxObjectsPerCell(1);
    CreateTestEntity(glm::vec3(0.0f, 0.0f, 0.0f));
    CreateTestEntity(glm::vec3(0.1f, 0.1f, 0.1f));
    EXPECT_NO_THROW(octree->Build());
    
    // Test with entities at extreme positions
    CreateTestEntity(glm::vec3(1000.0f, 1000.0f, 1000.0f));
    CreateTestEntity(glm::vec3(-1000.0f, -1000.0f, -1000.0f));
    EXPECT_NO_THROW(octree->Build());
}

// Test TreeNode with children
TEST_F(OctreeTest, TreeNodeWithChildren) 
{
    TreeNode parent(glm::vec3(0.0f), 10.0f, 0);
    TreeNode child(glm::vec3(5.0f), 5.0f, 1);
    
    // Manually set a child (normally done by octree construction)
    parent.pChildren[0] = &child;
    
    EXPECT_EQ(parent.pChildren[0], &child);
    EXPECT_EQ(child.level, parent.level + 1);
    EXPECT_EQ(child.halfwidth, parent.halfwidth / 2.0f);
}

// Test multiple builds (should handle cleanup properly)
TEST_F(OctreeTest, MultipleBuild) 
{
    // First build
    CreateTestEntity(glm::vec3(0.0f, 0.0f, 0.0f));
    octree->Build();
    
    // Add more entities and rebuild
    CreateTestEntity(glm::vec3(1.0f, 1.0f, 1.0f));
    CreateTestEntity(glm::vec3(-1.0f, -1.0f, -1.0f));
    octree->Build();
    
    // Build again with same entities
    octree->Build();
} 

// Test correct partitioning into eight octants when each octant has exactly one object
TEST_F(OctreeTest, OctreePartitioningEightOctants)
{
    octree->SetMaxObjectsPerCell(1);

    const float pos = 0.25f;
    const float neg = -0.25f;
    glm::vec3 positions[8] = {
        {neg, neg, neg},
        {pos, neg, neg},
        {neg, pos, neg},
        {pos, pos, neg},
        {neg, neg, pos},
        {pos, neg, pos},
        {neg, pos, pos},
        {pos, pos, pos}
    };

    for (const auto& p : positions)
    {
        CreateTestEntity(p, glm::vec3(0.1f));
    }

    octree->Build();

    const TreeNode* root = octree->GetRoot();
    ASSERT_NE(root, nullptr);

    for (int i = 0; i < 8; ++i)
    {
        const TreeNode* child = root->pChildren[i];
        ASSERT_NE(child, nullptr) << "Child " << i << " should exist";
        EXPECT_EQ(child->pObjects.size(), 1u) << "Child " << i << " should contain exactly one object";
    }
}

// Test that straddling objects stay at the parent level when using StayAtCurrentLevel method
TEST_F(OctreeTest, StraddlingStayAtCurrentLevel)
{
    octree->SetMaxObjectsPerCell(1);
    octree->SetStraddlingMethod(StraddlingMethod::StayAtCurrentLevel);

    auto straddleEntity = CreateTestEntity(glm::vec3(0.0f), glm::vec3(2.0f));
    auto otherEntity = CreateTestEntity(glm::vec3(3.0f, 3.0f, 3.0f), glm::vec3(0.1f));

    octree->Build();

    const TreeNode* root = octree->GetRoot();
    ASSERT_NE(root, nullptr);

    // Straddling entity should remain at root
    ASSERT_EQ(root->pObjects.size(), 1u);
    EXPECT_EQ(root->pObjects[0], straddleEntity);

    // Other entity should be located in one of the children
    bool foundOther = false;
    for (int i = 0; i < 8; ++i)
    {
        const TreeNode* child = root->pChildren[i];
        if (child && std::find(child->pObjects.begin(), child->pObjects.end(), otherEntity) != child->pObjects.end())
        {
            foundOther = true;
            break;
        }
    }
    EXPECT_TRUE(foundOther);
}

// Helper to gather maximum depth encountered in the tree
static void CollectMaxDepth(const TreeNode* node, int& maxDepth)
{
    if (!node) return;
    maxDepth = std::max(maxDepth, node->level);
    for (const auto child : node->pChildren)
    {
        CollectMaxDepth(child, maxDepth);
    }
}

// Ensure the tree depth never exceeds the configured maximum
TEST_F(OctreeTest, DepthDoesNotExceedLimit)
{
    // Create many entities to encourage deeper subdivision
    for (int x = -5; x <= 5; ++x)
        for (int y = -5; y <= 5; ++y)
            for (int z = -5; z <= 5; ++z)
            {
                CreateTestEntity(glm::vec3(x * 0.5f, y * 0.5f, z * 0.5f), glm::vec3(0.1f));
            }

    octree->Build();

    const TreeNode* root = octree->GetRoot();
    ASSERT_NE(root, nullptr);

    int maxDepth = 0;
    CollectMaxDepth(root, maxDepth);

    EXPECT_LE(maxDepth, 4);
}

// Test 12 objects with correct subdivision into octants
TEST_F(OctreeTest, TwelveObjectsCorrectSubdivision)
{
    octree->SetMaxObjectsPerCell(3); // This will force subdivision when we have 12 objects

    // Create 12 objects positioned to test all 8 octants
    // Each octant will have 1-2 objects to test proper distribution
    std::vector<Registry::Entity> entities;
    
    // Octant 0: (-x, -y, -z) - 2 objects
    entities.push_back(CreateTestEntity(glm::vec3(-0.75f, -0.75f, -0.75f), glm::vec3(0.1f)));
    entities.push_back(CreateTestEntity(glm::vec3(-0.25f, -0.25f, -0.25f), glm::vec3(0.1f)));
    
    // Octant 1: (+x, -y, -z) - 1 object
    entities.push_back(CreateTestEntity(glm::vec3(0.75f, -0.75f, -0.75f), glm::vec3(0.1f)));
    
    // Octant 2: (-x, +y, -z) - 1 object
    entities.push_back(CreateTestEntity(glm::vec3(-0.75f, 0.75f, -0.75f), glm::vec3(0.1f)));
    
    // Octant 3: (+x, +y, -z) - 2 objects
    entities.push_back(CreateTestEntity(glm::vec3(0.75f, 0.75f, -0.75f), glm::vec3(0.1f)));
    entities.push_back(CreateTestEntity(glm::vec3(0.25f, 0.25f, -0.25f), glm::vec3(0.1f)));
    
    // Octant 4: (-x, -y, +z) - 1 object
    entities.push_back(CreateTestEntity(glm::vec3(-0.75f, -0.75f, 0.75f), glm::vec3(0.1f)));
    
    // Octant 5: (+x, -y, +z) - 2 objects
    entities.push_back(CreateTestEntity(glm::vec3(0.75f, -0.75f, 0.75f), glm::vec3(0.1f)));
    entities.push_back(CreateTestEntity(glm::vec3(0.25f, -0.25f, 0.25f), glm::vec3(0.1f)));
    
    // Octant 6: (-x, +y, +z) - 2 objects
    entities.push_back(CreateTestEntity(glm::vec3(-0.75f, 0.75f, 0.75f), glm::vec3(0.1f)));
    entities.push_back(CreateTestEntity(glm::vec3(-0.25f, 0.25f, 0.25f), glm::vec3(0.1f)));
    
    // Octant 7: (+x, +y, +z) - 1 object
    entities.push_back(CreateTestEntity(glm::vec3(0.75f, 0.75f, 0.75f), glm::vec3(0.1f)));

    octree->Build();

    const TreeNode* root = octree->GetRoot();
    ASSERT_NE(root, nullptr);

    // Verify that all 8 children exist
    for (int i = 0; i < 8; ++i)
    {
        ASSERT_NE(root->pChildren[i], nullptr) << "Child " << i << " should exist";
    }

    // Verify object counts in each octant
    EXPECT_EQ(root->pChildren[0]->pObjects.size(), 2u) << "Octant 0 should have 2 objects";
    EXPECT_EQ(root->pChildren[1]->pObjects.size(), 1u) << "Octant 1 should have 1 object";
    EXPECT_EQ(root->pChildren[2]->pObjects.size(), 1u) << "Octant 2 should have 1 object";
    EXPECT_EQ(root->pChildren[3]->pObjects.size(), 2u) << "Octant 3 should have 2 objects";
    EXPECT_EQ(root->pChildren[4]->pObjects.size(), 1u) << "Octant 4 should have 1 object";
    EXPECT_EQ(root->pChildren[5]->pObjects.size(), 2u) << "Octant 5 should have 2 objects";
    EXPECT_EQ(root->pChildren[6]->pObjects.size(), 2u) << "Octant 6 should have 2 objects";
    EXPECT_EQ(root->pChildren[7]->pObjects.size(), 1u) << "Octant 7 should have 1 object";

    // Verify total object count
    size_t totalObjects = 0;
    for (int i = 0; i < 8; ++i)
    {
        totalObjects += root->pChildren[i]->pObjects.size();
    }
    EXPECT_EQ(totalObjects, 12u) << "Total objects should be 12";
}

// Test 12 objects with straddling objects using different methods
TEST_F(OctreeTest, TwelveObjectsWithStraddling)
{
    octree->SetMaxObjectsPerCell(2); // Force subdivision with fewer objects per cell

    std::vector<Registry::Entity> entities;
    
    // Create 8 objects in distinct octants
    entities.push_back(CreateTestEntity(glm::vec3(-0.75f, -0.75f, -0.75f), glm::vec3(0.1f)));
    entities.push_back(CreateTestEntity(glm::vec3(0.75f, -0.75f, -0.75f), glm::vec3(0.1f)));
    entities.push_back(CreateTestEntity(glm::vec3(-0.75f, 0.75f, -0.75f), glm::vec3(0.1f)));
    entities.push_back(CreateTestEntity(glm::vec3(0.75f, 0.75f, -0.75f), glm::vec3(0.1f)));
    entities.push_back(CreateTestEntity(glm::vec3(-0.75f, -0.75f, 0.75f), glm::vec3(0.1f)));
    entities.push_back(CreateTestEntity(glm::vec3(0.75f, -0.75f, 0.75f), glm::vec3(0.1f)));
    entities.push_back(CreateTestEntity(glm::vec3(-0.75f, 0.75f, 0.75f), glm::vec3(0.1f)));
    entities.push_back(CreateTestEntity(glm::vec3(0.75f, 0.75f, 0.75f), glm::vec3(0.1f)));
    
    // Add 4 straddling objects that cross octant boundaries
    entities.push_back(CreateTestEntity(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f))); // Center straddler
    entities.push_back(CreateTestEntity(glm::vec3(0.5f, 0.0f, 0.0f), glm::vec3(0.8f))); // X-axis straddler
    entities.push_back(CreateTestEntity(glm::vec3(0.0f, 0.5f, 0.0f), glm::vec3(0.8f))); // Y-axis straddler
    entities.push_back(CreateTestEntity(glm::vec3(0.0f, 0.0f, 0.5f), glm::vec3(0.8f))); // Z-axis straddler

    // Test with UseCenter method
    octree->SetStraddlingMethod(StraddlingMethod::UseCenter);
    octree->Build();

    const TreeNode* root = octree->GetRoot();
    ASSERT_NE(root, nullptr);

    // With UseCenter, straddling objects should be placed in specific octants based on their center
    size_t totalObjectsInChildren = 0;
    for (int i = 0; i < 8; ++i)
    {
        totalObjectsInChildren += root->pChildren[i]->pObjects.size();
    }
    
    // All 12 objects should be distributed among children
    EXPECT_EQ(totalObjectsInChildren, 12u) << "All 12 objects should be in children with UseCenter method";
}

// Test 12 objects with deep subdivision
TEST_F(OctreeTest, TwelveObjectsDeepSubdivision)
{
    octree->SetMaxObjectsPerCell(1); // Force maximum subdivision

    std::vector<Registry::Entity> entities;
    
    // Create 12 objects in a 2x2x3 grid pattern to test deep subdivision
    for (int x = 0; x < 2; ++x)
    {
        for (int y = 0; y < 2; ++y)
        {
            for (int z = 0; z < 3; ++z)
            {
                float xPos = (x - 0.5f) * 0.5f;
                float yPos = (y - 0.5f) * 0.5f;
                float zPos = (z - 1.0f) * 0.5f;
                entities.push_back(CreateTestEntity(glm::vec3(xPos, yPos, zPos), glm::vec3(0.05f)));
            }
        }
    }

    octree->Build();

    const TreeNode* root = octree->GetRoot();
    ASSERT_NE(root, nullptr);

    // Count total objects in the tree
    size_t totalObjects = 0;
    std::function<void(const TreeNode*)> countObjects = [&](const TreeNode* node) {
        if (!node) return;
        totalObjects += node->pObjects.size();
        for (const auto child : node->pChildren)
        {
            countObjects(child);
        }
    };
    
    countObjects(root);
    EXPECT_EQ(totalObjects, 12u) << "Total objects in tree should be 12";

    // Verify that some nodes have children (indicating subdivision occurred)
    bool hasSubdivision = false;
    std::function<void(const TreeNode*)> checkSubdivision = [&](const TreeNode* node) {
        if (!node) return;
        for (const auto child : node->pChildren)
        {
            if (child)
            {
                hasSubdivision = true;
                checkSubdivision(child);
            }
        }
    };
    
    checkSubdivision(root);
    EXPECT_TRUE(hasSubdivision) << "Tree should have subdivisions with maxObjectsPerCell=1";
}

// Test 12 objects with boundary conditions
TEST_F(OctreeTest, TwelveObjectsBoundaryConditions)
{
    octree->SetMaxObjectsPerCell(3);

    std::vector<Registry::Entity> entities;
    
    // Create objects at the very edges of octants to test boundary conditions
    // Octant boundaries are at x=0, y=0, z=0
    
    // Objects just inside each octant
    entities.push_back(CreateTestEntity(glm::vec3(-0.01f, -0.01f, -0.01f), glm::vec3(0.01f))); // Just inside octant 0
    entities.push_back(CreateTestEntity(glm::vec3(0.01f, -0.01f, -0.01f), glm::vec3(0.01f)));  // Just inside octant 1
    entities.push_back(CreateTestEntity(glm::vec3(-0.01f, 0.01f, -0.01f), glm::vec3(0.01f)));  // Just inside octant 2
    entities.push_back(CreateTestEntity(glm::vec3(0.01f, 0.01f, -0.01f), glm::vec3(0.01f)));   // Just inside octant 3
    entities.push_back(CreateTestEntity(glm::vec3(-0.01f, -0.01f, 0.01f), glm::vec3(0.01f)));  // Just inside octant 4
    entities.push_back(CreateTestEntity(glm::vec3(0.01f, -0.01f, 0.01f), glm::vec3(0.01f)));   // Just inside octant 5
    entities.push_back(CreateTestEntity(glm::vec3(-0.01f, 0.01f, 0.01f), glm::vec3(0.01f)));   // Just inside octant 6
    entities.push_back(CreateTestEntity(glm::vec3(0.01f, 0.01f, 0.01f), glm::vec3(0.01f)));    // Just inside octant 7
    
    // Objects at the center of each octant
    entities.push_back(CreateTestEntity(glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(0.1f))); // Center of octant 0
    entities.push_back(CreateTestEntity(glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(0.1f)));  // Center of octant 1
    entities.push_back(CreateTestEntity(glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec3(0.1f)));  // Center of octant 2
    entities.push_back(CreateTestEntity(glm::vec3(0.5f, 0.5f, -0.5f), glm::vec3(0.1f)));   // Center of octant 3

    octree->Build();

    const TreeNode* root = octree->GetRoot();
    ASSERT_NE(root, nullptr);

    // Verify that all 8 children exist
    for (int i = 0; i < 8; ++i)
    {
        ASSERT_NE(root->pChildren[i], nullptr) << "Child " << i << " should exist";
    }

    // Each octant should have at least one object
    for (int i = 0; i < 8; ++i)
    {
        EXPECT_GT(root->pChildren[i]->pObjects.size(), 0u) << "Octant " << i << " should have at least one object";
    }

    // Verify total object count
    size_t totalObjects = 0;
    for (int i = 0; i < 8; ++i)
    {
        totalObjects += root->pChildren[i]->pObjects.size();
    }
    EXPECT_EQ(totalObjects, 12u) << "Total objects should be 12";
}

// Test 12 objects with mixed straddling methods
TEST_F(OctreeTest, TwelveObjectsMixedStraddlingMethods)
{
    octree->SetMaxObjectsPerCell(2);

    std::vector<Registry::Entity> entities;
    
    // Create 8 objects in distinct octants
    for (int i = 0; i < 8; ++i)
    {
        float x = (i & 1) ? 0.75f : -0.75f;
        float y = (i & 2) ? 0.75f : -0.75f;
        float z = (i & 4) ? 0.75f : -0.75f;
        entities.push_back(CreateTestEntity(glm::vec3(x, y, z), glm::vec3(0.1f)));
    }
    
    // Add 4 straddling objects
    entities.push_back(CreateTestEntity(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.5f))); // Large center straddler
    entities.push_back(CreateTestEntity(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.8f))); // Medium center straddler
    entities.push_back(CreateTestEntity(glm::vec3(0.5f, 0.0f, 0.0f), glm::vec3(0.6f))); // X-axis straddler
    entities.push_back(CreateTestEntity(glm::vec3(0.0f, 0.5f, 0.0f), glm::vec3(0.6f))); // Y-axis straddler

    // StayAtCurrentLevel already tested above
    octree->SetStraddlingMethod(StraddlingMethod::StayAtCurrentLevel);
    octree->Build();

    const TreeNode* root = octree->GetRoot();
    ASSERT_NE(root, nullptr);

    // With StayAtCurrentLevel, straddling objects should be in multiple octants
    size_t totalObjectsInChildren = 0;
    for (int i = 0; i < 8; ++i)
    {
        totalObjectsInChildren += root->pChildren[i]->pObjects.size();
    }
    
    // Total should be more than 12 due to straddling objects being in multiple octants
    EXPECT_GT(totalObjectsInChildren, 12u) << "Total objects should be more than 12 with StayAtCurrentLevel method";
} 