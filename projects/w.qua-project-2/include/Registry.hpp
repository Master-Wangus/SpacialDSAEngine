/**
 * @class Registry
 * @brief Central registry for entity-component management.
 *
 * This class implements a centralized registry for entity-component system (ECS) architecture,
 * handling entity creation, component attachment, and system registration.
 */

#pragma once

#include "pch.h"

class Registry
{
public:
    using Entity = entt::entity;
    
    Registry() = default;
    
    /**
     * @brief Creates a new entity in the registry.
     * @return Handle to the newly created entity
     */
    Entity Create();
    
    /**
     * @brief Destroys an entity and all its components.
     * @param entity Entity to destroy
     */
    void Destroy(Entity entity);
    
    /**
     * @brief Adds a component to an entity.
     * @tparam T Component type to add
     * @tparam Args Constructor argument types
     * @param entity Entity to add component to
     * @param args Constructor arguments for the component
     * @return Reference to the added component
     */
    template<typename T, typename... Args>
    T& AddComponent(Entity entity, Args&&... args) 
    {
        return m_Registry.emplace<T>(entity, std::forward<Args>(args)...);
    }
    
    /**
     * @brief Gets a component from an entity.
     * @tparam T Component type to retrieve
     * @param entity Entity to get component from
     * @return Reference to the component
     */
    template<typename T>
    T& GetComponent(Entity entity) 
    {
        return m_Registry.get<T>(entity);
    }
    
    /**
     * @brief Checks if an entity has a specific component.
     * @tparam T Component type to check for
     * @param entity Entity to check
     * @return True if entity has the component, false otherwise
     */
    template<typename T>
    bool HasComponent(Entity entity) const 
    {
        return m_Registry.all_of<T>(entity);
    }
    
    /**
     * @brief Removes a component from an entity.
     * @tparam T Component type to remove
     * @param entity Entity to remove component from
     */
    template<typename T>
    void RemoveComponent(Entity entity) 
    {
        m_Registry.remove<T>(entity);
    }
    
    /**
     * @brief Creates a view of entities with specific components.
     * @tparam Components Component types to include in the view
     * @return View object for iterating over matching entities
     */
    template<typename... Components>
    auto View() 
    {
        return m_Registry.view<Components...>();
    }
    
    /**
     * @brief Gets the underlying EnTT registry.
     * @return Reference to the EnTT registry
     */
    entt::registry& GetRegistry() 
    {
        return m_Registry;
    }
    
    /**
     * @brief Gets the underlying EnTT registry (const version).
     * @return Const reference to the EnTT registry
     */
    const entt::registry& GetRegistry() const 
    {
        return m_Registry;
    }

private:
    entt::registry m_Registry;
}; 