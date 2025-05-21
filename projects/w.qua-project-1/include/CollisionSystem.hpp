#pragma once

#include "Components.hpp"
#include "Registry.hpp"
#include <vector>

struct CollisionPair {
    Registry::Entity entity1;
    Registry::Entity entity2;
};

class CollisionSystem {
public:
    CollisionSystem(Registry& registry);
    
    // Detect all collisions and store the results
    void DetectCollisions();
    
    // Update all collider positions based on transforms
    void UpdateColliders();
    
    // Get the list of collisions that occurred in the last frame
    const std::vector<CollisionPair>& GetCollisions() const;

private:
    // Specific collision tests
    bool TestAABBvsAABB(const AABB& a, const AABB& b);
    bool TestSphereVsSphere(const BoundingSphere& a, const BoundingSphere& b);
    
    Registry& m_Registry;
    std::vector<CollisionPair> m_Collisions;
}; 