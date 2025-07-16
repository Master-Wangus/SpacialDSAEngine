#include <gtest/gtest.h>
#include "Geometry.hpp"
#include "Buffer.hpp"

class GeometryTest : public ::testing::Test 
{
protected:
    void SetUp() override 
    {        
        // Simple plane: z = 0 (XY plane)
        plane_normal = glm::vec3(0.0f, 0.0f, 1.0f);
        plane_distance = 0.0f;
        
        // AABB test cases
        aabb_behind_min = Vertex{glm::vec3(-1.0f, -1.0f, -2.0f), glm::vec3(1.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f)};
        aabb_behind_max = Vertex{glm::vec3(1.0f, 1.0f, -1.0f), glm::vec3(1.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f)};
        
        aabb_front_min = Vertex{glm::vec3(-1.0f, -1.0f, 1.0f), glm::vec3(1.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f)};
        aabb_front_max = Vertex{glm::vec3(1.0f, 1.0f, 2.0f), glm::vec3(1.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f)};
        
        aabb_straddle_min = Vertex{glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3(1.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f)};
        aabb_straddle_max = Vertex{glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(1.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f)};
        
        // Sphere test cases
        sphere_behind = Vertex{glm::vec3(0.0f, 0.0f, -2.0f), glm::vec3(1.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f)};
        sphere_front = Vertex{glm::vec3(0.0f, 0.0f, 2.0f), glm::vec3(1.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f)};
        sphere_on_plane = Vertex{glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f)};
        
        sphere_radius_small = 0.5f;
        sphere_radius_large = 1.5f;
        
        // Simple frustum setup (looking down negative Z axis)
        setupSimpleFrustum();
    }
    
    void setupSimpleFrustum() 
    {
        
        frustum_normals[0] = glm::normalize(glm::vec3(1.0f, 0.0f, 1.0f));   // Left
        frustum_normals[1] = glm::normalize(glm::vec3(-1.0f, 0.0f, 1.0f));  // Right
        frustum_normals[2] = glm::normalize(glm::vec3(0.0f, 1.0f, 1.0f));   // Bottom
        frustum_normals[3] = glm::normalize(glm::vec3(0.0f, -1.0f, 1.0f));  // Top
        frustum_normals[4] = glm::vec3(0.0f, 0.0f, 1.0f);   // Near
        frustum_normals[5] = glm::vec3(0.0f, 0.0f, -1.0f);  // Far
        
        frustum_distances[0] = 0.0f;  // Left
        frustum_distances[1] = 0.0f;  // Right
        frustum_distances[2] = 0.0f;  // Bottom
        frustum_distances[3] = 0.0f;  // Top
        frustum_distances[4] = 1.0f;  // Near
        frustum_distances[5] = 10.0f; // Far
    }

    glm::vec3 plane_normal;
    float plane_distance;
    
    Vertex aabb_behind_min, aabb_behind_max;
    Vertex aabb_front_min, aabb_front_max;
    Vertex aabb_straddle_min, aabb_straddle_max;
    
    Vertex sphere_behind, sphere_front, sphere_on_plane;
    float sphere_radius_small, sphere_radius_large;
    
    glm::vec3 frustum_normals[6];
    float frustum_distances[6];
};

// Plane-AABB Classification Tests
TEST_F(GeometryTest, ClassifyPlaneAabbBehind) 
{
    SideResult result = ClassifyPlaneAabb(plane_normal, plane_distance, 
                                          aabb_behind_min, aabb_behind_max);
    EXPECT_EQ(result, SideResult::eINSIDE);
}

TEST_F(GeometryTest, ClassifyPlaneAabbFront) 
{
    SideResult result = ClassifyPlaneAabb(plane_normal, plane_distance, 
                                          aabb_front_min, aabb_front_max);
    EXPECT_EQ(result, SideResult::eOUTSIDE);
}

TEST_F(GeometryTest, ClassifyPlaneAabbStraddling) 
{
    SideResult result = ClassifyPlaneAabb(plane_normal, plane_distance, 
                                          aabb_straddle_min, aabb_straddle_max);
    EXPECT_EQ(result, SideResult::eOVERLAPPING);
}

// Plane-Sphere Classification Tests
TEST_F(GeometryTest, ClassifyPlaneSphereBehind) 
{
    SideResult result = ClassifyPlaneSphere(plane_normal, plane_distance, 
                                            sphere_behind, sphere_radius_small);
    EXPECT_EQ(result, SideResult::eINSIDE);
}

TEST_F(GeometryTest, ClassifyPlaneSphereFront) 
{
    SideResult result = ClassifyPlaneSphere(plane_normal, plane_distance, 
                                            sphere_front, sphere_radius_small);
    EXPECT_EQ(result, SideResult::eOUTSIDE);
}

TEST_F(GeometryTest, ClassifyPlaneSphereOnPlane) 
{
    SideResult result = ClassifyPlaneSphere(plane_normal, plane_distance, 
                                            sphere_on_plane, sphere_radius_small);
    EXPECT_EQ(result, SideResult::eOVERLAPPING);
}

TEST_F(GeometryTest, ClassifyPlaneSphereStraddling) 
{
    SideResult result = ClassifyPlaneSphere(plane_normal, plane_distance, 
                                            sphere_on_plane, sphere_radius_large);
    EXPECT_EQ(result, SideResult::eOVERLAPPING);
}

// Frustum-Sphere Classification Tests
TEST_F(GeometryTest, ClassifyFrustumSphereInside) 
{
    Vertex center{glm::vec3(0.0f, 0.0f, -5.0f), glm::vec3(1.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f)};
    float radius = 1.0f;
    
    SideResult result = ClassifyFrustumSphereNaive(frustum_normals, frustum_distances, 
                                                   center, radius);
    EXPECT_EQ(result, SideResult::eINSIDE);
}

TEST_F(GeometryTest, ClassifyFrustumSphereOutside) 
{
    // Sphere way outside frustum
    Vertex center{glm::vec3(20.0f, 0.0f, -5.0f), glm::vec3(1.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f)};
    float radius = 1.0f;
    
    SideResult result = ClassifyFrustumSphereNaive(frustum_normals, frustum_distances, 
                                                   center, radius);
    EXPECT_EQ(result, SideResult::eOUTSIDE);
}

// Frustum-AABB Classification Tests
TEST_F(GeometryTest, ClassifyFrustumAabbInside) 
{
    Vertex min{glm::vec3(-1.0f, -1.0f, -6.0f), glm::vec3(1.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f)};
    Vertex max{glm::vec3(1.0f, 1.0f, -4.0f), glm::vec3(1.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f)};
    
    SideResult result = ClassifyFrustumAabbNaive(frustum_normals, frustum_distances, 
                                                 min, max);
    EXPECT_EQ(result, SideResult::eINSIDE);
}

TEST_F(GeometryTest, ClassifyFrustumAabbOutside)
{
    // AABB way outside frustum
    Vertex min{glm::vec3(20.0f, 20.0f, -6.0f), glm::vec3(1.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f)};
    Vertex max{glm::vec3(22.0f, 22.0f, -4.0f), glm::vec3(1.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f)};
    
    SideResult result = ClassifyFrustumAabbNaive(frustum_normals, frustum_distances, 
                                                 min, max);
    EXPECT_EQ(result, SideResult::eOUTSIDE);
}

// AABB Transform Tests
TEST_F(GeometryTest, TransformAabbIdentity)
{
    glm::vec3 min_orig(-1.0f, -1.0f, -1.0f);
    glm::vec3 max_orig(1.0f, 1.0f, 1.0f);
    glm::vec3 min = min_orig;
    glm::vec3 max = max_orig;
    
    glm::mat4 identity(1.0f);
    TransformAabb(min, max, identity);
    
    EXPECT_EQ(min, min_orig);
    EXPECT_EQ(max, max_orig);
}

TEST_F(GeometryTest, TransformAabbTranslation) 
{
    glm::vec3 min(-1.0f, -1.0f, -1.0f);
    glm::vec3 max(1.0f, 1.0f, 1.0f);
    
    glm::mat4 translation = glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, 3.0f, 4.0f));
    TransformAabb(min, max, translation);
    
    EXPECT_NEAR(min.x, 1.0f, 0.001f);  // -1 + 2 = 1
    EXPECT_NEAR(min.y, 2.0f, 0.001f);  // -1 + 3 = 2
    EXPECT_NEAR(min.z, 3.0f, 0.001f);  // -1 + 4 = 3
    
    EXPECT_NEAR(max.x, 3.0f, 0.001f);  // 1 + 2 = 3
    EXPECT_NEAR(max.y, 4.0f, 0.001f);  // 1 + 3 = 4
    EXPECT_NEAR(max.z, 5.0f, 0.001f);  // 1 + 4 = 5
}

// AABB Creation Tests
TEST_F(GeometryTest, CreateAabbBruteForce) 
{
    std::vector<Vertex> vertices = {
        {glm::vec3(-2.0f, -3.0f, -4.0f), glm::vec3(1.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f)},
        {glm::vec3(5.0f, 1.0f, 2.0f), glm::vec3(1.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f)},
        {glm::vec3(0.0f, 4.0f, -1.0f), glm::vec3(1.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f)},
        {glm::vec3(-1.0f, -1.0f, 3.0f), glm::vec3(1.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f)}
    };
    
    Vertex min, max;
    CreateAabbBruteForce(vertices.data(), vertices.size(), &min, &max);
    
    EXPECT_EQ(min.m_Position, glm::vec3(-2.0f, -3.0f, -4.0f));
    EXPECT_EQ(max.m_Position, glm::vec3(5.0f, 4.0f, 3.0f));
}

// Sphere Creation Tests
TEST_F(GeometryTest, CreateSpherePCA) 
{
    std::vector<Vertex> vertices = {
        {glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(1.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f)},
        {glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(1.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f)},
        {glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(1.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f)},
        {glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f)},
        {glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(1.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f)},
        {glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(1.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f)}
    };
    
    Vertex center;
    float radius;
    CreateSpherePCA(vertices.data(), vertices.size(), &center, &radius);
    
    // Center should be approximately at origin
    EXPECT_NEAR(center.m_Position.x, 0.0f, 0.1f);
    EXPECT_NEAR(center.m_Position.y, 0.0f, 0.1f);
    EXPECT_NEAR(center.m_Position.z, 0.0f, 0.1f);
    
    // Radius should be approximately 1.0 (distance to vertices)
    EXPECT_NEAR(radius, 1.0f, 0.1f);
}

// OBB Creation Tests
TEST_F(GeometryTest, CreateObbPCA) 
{
    // Create vertices aligned with axes for predictable result
    std::vector<Vertex> vertices = 
    {
        {glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(1.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f)},
        {glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(1.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f)},
        {glm::vec3(0.0f, -2.0f, 0.0f), glm::vec3(1.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f)},
        {glm::vec3(0.0f, 2.0f, 0.0f), glm::vec3(1.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f)},
        {glm::vec3(0.0f, 0.0f, -3.0f), glm::vec3(1.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f)},
        {glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(1.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f)}
    };
    
    glm::vec3 center;
    glm::vec3 axes[3];
    glm::vec3 halfExtents;
    
    CreateObbPCA(vertices.data(), vertices.size(), &center, axes, &halfExtents);
    
    // Center should be approximately at origin
    EXPECT_NEAR(center.x, 0.0f, 0.1f);
    EXPECT_NEAR(center.y, 0.0f, 0.1f);
    EXPECT_NEAR(center.z, 0.0f, 0.1f);
    
    // Half-extents should be approximately 1, 2, 3
    std::vector<float> extents = {halfExtents.x, halfExtents.y, halfExtents.z};
    std::sort(extents.begin(), extents.end());
    
    EXPECT_NEAR(extents[0], 1.0f, 0.1f);
    EXPECT_NEAR(extents[1], 2.0f, 0.1f);
    EXPECT_NEAR(extents[2], 3.0f, 0.1f);
}

// Edge Cases
TEST_F(GeometryTest, ClassifyPlaneAabbOnPlane) 
{
    // AABB that lies exactly on the plane
    Vertex min{glm::vec3(-1.0f, -1.0f, 0.0f), glm::vec3(1.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f)};
    Vertex max{glm::vec3(1.0f, 1.0f, 0.0f), glm::vec3(1.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f)};
    
    SideResult result = ClassifyPlaneAabb(plane_normal, plane_distance, min, max);
    // An AABB that lies exactly on the plane should be classified as overlapping
    EXPECT_EQ(result, SideResult::eOVERLAPPING);
}

TEST_F(GeometryTest, ClassifyPlaneAabbPartialOnPlane)
{
    // AABB that partially lies on the plane (some vertices on plane, some behind)
    Vertex min{glm::vec3(-1.0f, -1.0f, -0.5f), glm::vec3(1.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f)};
    Vertex max{glm::vec3(1.0f, 1.0f, 0.0f), glm::vec3(1.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f)};
    
    SideResult result = ClassifyPlaneAabb(plane_normal, plane_distance, min, max);
    // AABB with some vertices on plane and some behind should be overlapping
    EXPECT_EQ(result, SideResult::eOVERLAPPING);
}

TEST_F(GeometryTest, CreateAabbSingleVertex) 
{
    Vertex vertex{glm::vec3(1.0f, 2.0f, 3.0f), glm::vec3(1.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f)};
    Vertex min, max;
    
    CreateAabbBruteForce(&vertex, 1, &min, &max);
    
    EXPECT_EQ(min.m_Position, glm::vec3(1.0f, 2.0f, 3.0f));
    EXPECT_EQ(max.m_Position, glm::vec3(1.0f, 2.0f, 3.0f));
} 