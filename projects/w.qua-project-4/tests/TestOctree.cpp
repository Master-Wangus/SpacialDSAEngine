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
    octree->SetStraddlingMethod(StraddlingMethod::AssociateAll);
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
    
    // Test AssociateAll method
    octree->SetStraddlingMethod(StraddlingMethod::AssociateAll);
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
    octree->SetStraddlingMethod(StraddlingMethod::AssociateAll);
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