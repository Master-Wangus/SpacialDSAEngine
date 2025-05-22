#pragma once

#include "pch.h"
#include "Registry.hpp"
#include "Components.hpp"
#include "Primitives.hpp"
#include "Intersection.hpp"

struct CollisionPair 
{
    Registry::Entity entity1;
    Registry::Entity entity2;
};

class CollisionSystem 
{
public:
    CollisionSystem(Registry& registry);
    
    // Detect all collisions and store the results
    void DetectCollisions();
    
    // Update all collider positions based on transforms
    void UpdateColliders();
    
    // Get the list of collisions that occurred in the last frame
    const std::vector<CollisionPair>& GetCollisions() const;

private:
    Registry& m_Registry;
    std::vector<CollisionPair> m_Collisions;
    
    // Helper method to check if a collision already exists
    bool CollisionExists(Registry::Entity entity1, Registry::Entity entity2) const;
    
    // Helper method for collision detection between different shape types
    bool CheckCollision(const CollisionComponent& a, const CollisionComponent& b);
}; 