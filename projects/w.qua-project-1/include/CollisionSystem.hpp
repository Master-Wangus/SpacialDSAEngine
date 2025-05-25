/**
 * @class CollisionSystem
 * @brief System for detecting and handling collisions between 3D objects.
 *
 * This system processes collision components and performs intersection tests between
 * different 3D primitives, providing collision response and visualization.
 */

#pragma once

#include "pch.h"
#include "Registry.hpp"
#include "Components.hpp"

struct CollisionPair 
{
    Registry::Entity entity1;
    Registry::Entity entity2;
    
    bool Contains(Registry::Entity a, Registry::Entity b) const 
    {
        return (entity1 == a && entity2 == b) || (entity1 == b && entity2 == a);
    }
};

// Function type for collision test handlers
using CollisionTestFunction = std::function<bool(const CollisionComponent&, const CollisionComponent&)>;

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
    
    std::map<std::pair<CollisionShapeType, CollisionShapeType>, CollisionTestFunction> m_CollisionTests;
    
    void InitializeCollisionTests();
    bool CheckCollision(const CollisionComponent& a, const CollisionComponent& b);    
    bool HasCollision(Registry::Entity entity1, Registry::Entity entity2);
}; 