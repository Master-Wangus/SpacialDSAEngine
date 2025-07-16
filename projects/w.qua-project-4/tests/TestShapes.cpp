#include <gtest/gtest.h>
#include "Shapes.hpp"

class ShapesTest : public ::testing::Test 
{
protected:
    void SetUp() override 
    {

        aabb1 = Aabb(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(2.0f, 2.0f, 2.0f));
        aabb2 = Aabb(glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(3.0f, 3.0f, 3.0f));
        aabb3 = Aabb(glm::vec3(5.0f, 5.0f, 5.0f), glm::vec3(7.0f, 7.0f, 7.0f));
        
        sphere1 = Sphere(glm::vec3(0.0f, 0.0f, 0.0f), 1.0f);
        sphere2 = Sphere(glm::vec3(2.0f, 0.0f, 0.0f), 1.5f);
    }

    Aabb aabb1, aabb2, aabb3;
    Sphere sphere1, sphere2;
};

// AABB Tests
TEST_F(ShapesTest, AabbDefaultConstructor) 
{
    Aabb aabb;
    EXPECT_EQ(aabb.min, glm::vec3(0.0f, 0.0f, 0.0f));
    EXPECT_EQ(aabb.max, glm::vec3(0.0f, 0.0f, 0.0f));
}

TEST_F(ShapesTest, AabbMinMaxConstructor) 
{
    glm::vec3 min(-1.0f, -2.0f, -3.0f);
    glm::vec3 max(1.0f, 2.0f, 3.0f);
    Aabb aabb(min, max);
    
    EXPECT_EQ(aabb.min, min);
    EXPECT_EQ(aabb.max, max);
}

TEST_F(ShapesTest, AabbCenterExtentsConstructor) 
{
    glm::vec3 center(1.0f, 2.0f, 3.0f);
    float extents = 2.0f;
    Aabb aabb(center, extents);
    
    EXPECT_EQ(aabb.GetCenter(), center);
    glm::vec3 expectedExtents(extents, extents, extents);
    EXPECT_EQ(aabb.GetExtents(), expectedExtents);
}

TEST_F(ShapesTest, AabbGetCenter) 
{
    glm::vec3 expectedCenter(1.0f, 1.0f, 1.0f);
    EXPECT_EQ(aabb1.GetCenter(), expectedCenter);
}

TEST_F(ShapesTest, AabbGetExtents) 
{
    glm::vec3 expectedExtents(1.0f, 1.0f, 1.0f);
    EXPECT_EQ(aabb1.GetExtents(), expectedExtents);
}

TEST_F(ShapesTest, AabbSetCenter) 
{
    glm::vec3 newCenter(5.0f, 5.0f, 5.0f);
    glm::vec3 originalExtents = aabb1.GetExtents();
    
    aabb1.SetCenter(newCenter);
    
    EXPECT_EQ(aabb1.GetCenter(), newCenter);
    EXPECT_EQ(aabb1.GetExtents(), originalExtents);
}

TEST_F(ShapesTest, AabbOverlapsTrue) 
{
    // aabb1 and aabb2 should overlap
    EXPECT_TRUE(aabb1.Overlaps(aabb2));
    EXPECT_TRUE(aabb2.Overlaps(aabb1));
}

TEST_F(ShapesTest, AabbOverlapsFalse) 
{
    // aabb1 and aabb3 should not overlap
    EXPECT_FALSE(aabb1.Overlaps(aabb3));
    EXPECT_FALSE(aabb3.Overlaps(aabb1));
}

TEST_F(ShapesTest, AabbOverlapsSelf) 
{
    EXPECT_TRUE(aabb1.Overlaps(aabb1));
}

TEST_F(ShapesTest, AabbTransform) 
{
    glm::mat4 translation = glm::translate(glm::mat4(1.0f), glm::vec3(1.0f, 2.0f, 3.0f));
    glm::vec3 originalCenter = aabb1.GetCenter();
    
    aabb1.Transform(translation);
    
    glm::vec3 expectedNewCenter = originalCenter + glm::vec3(1.0f, 2.0f, 3.0f);
    
    // Check that center moved correctly (allowing for small floating point errors)
    glm::vec3 actualCenter = aabb1.GetCenter();
    EXPECT_NEAR(actualCenter.x, expectedNewCenter.x, 0.001f);
    EXPECT_NEAR(actualCenter.y, expectedNewCenter.y, 0.001f);
    EXPECT_NEAR(actualCenter.z, expectedNewCenter.z, 0.001f);
}

// Sphere Tests
TEST_F(ShapesTest, SphereDefaultConstructor) 
{
    Sphere sphere;
    EXPECT_EQ(sphere.center, glm::vec3(0.0f, 0.0f, 0.0f));
    EXPECT_EQ(sphere.radius, 0.0f);
}

TEST_F(ShapesTest, SphereParametrizedConstructor) 
{
    glm::vec3 center(1.0f, 2.0f, 3.0f);
    float radius = 5.0f;
    
    Sphere sphere(center, radius);
    
    EXPECT_EQ(sphere.center, center);
    EXPECT_EQ(sphere.radius, radius);
}

// OBB Tests
TEST_F(ShapesTest, ObbDefaultConstructor) 
{
    Obb obb;
    
    EXPECT_EQ(obb.center, glm::vec3(0.0f, 0.0f, 0.0f));
    EXPECT_EQ(obb.halfExtents, glm::vec3(0.0f, 0.0f, 0.0f));
    
    // Check default axes (identity)
    EXPECT_EQ(obb.axes[0], glm::vec3(1.0f, 0.0f, 0.0f));
    EXPECT_EQ(obb.axes[1], glm::vec3(0.0f, 1.0f, 0.0f));
    EXPECT_EQ(obb.axes[2], glm::vec3(0.0f, 0.0f, 1.0f));
}

TEST_F(ShapesTest, ObbParametrizedConstructor) 
{
    glm::vec3 center(1.0f, 2.0f, 3.0f);
    glm::vec3 axes[3] = {
        glm::vec3(1.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 1.0f)
    };
    glm::vec3 halfExtents(2.0f, 3.0f, 4.0f);
    
    Obb obb(center, axes, halfExtents);
    
    EXPECT_EQ(obb.center, center);
    EXPECT_EQ(obb.halfExtents, halfExtents);
    EXPECT_EQ(obb.axes[0], axes[0]);
    EXPECT_EQ(obb.axes[1], axes[1]);
    EXPECT_EQ(obb.axes[2], axes[2]);
}

// Edge case tests
TEST_F(ShapesTest, AabbZeroSize) 
{
    Aabb point(glm::vec3(1.0f, 2.0f, 3.0f), glm::vec3(1.0f, 2.0f, 3.0f));
    
    EXPECT_EQ(point.GetExtents(), glm::vec3(0.0f, 0.0f, 0.0f));
    EXPECT_EQ(point.GetCenter(), glm::vec3(1.0f, 2.0f, 3.0f));
}

TEST_F(ShapesTest, SphereZeroRadius) 
{
    Sphere point(glm::vec3(1.0f, 2.0f, 3.0f), 0.0f);
    
    EXPECT_EQ(point.center, glm::vec3(1.0f, 2.0f, 3.0f));
    EXPECT_EQ(point.radius, 0.0f);
} 