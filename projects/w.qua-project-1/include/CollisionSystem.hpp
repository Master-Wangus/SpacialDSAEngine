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
    
    void DetectCollisions();
    
    void UpdateColliders();
    
    const std::vector<CollisionPair>& GetCollisions() const;

private:
    Registry& m_Registry;
    std::vector<CollisionPair> m_Collisions;
    
    bool CheckCollision(const CollisionComponent& a, const CollisionComponent& b);
}; 