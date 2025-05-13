#pragma once

#include <entt/entt.hpp>
#include <memory>

class Registry
 {
public:
    using Entity = entt::entity;
    Registry() = default;
    Entity create();
    void destroy(Entity entity);
    
    template<typename T, typename... Args>
    T& addComponent(Entity entity, Args&&... args) 
    {
        return m_registry.emplace<T>(entity, std::forward<Args>(args)...);
    }
    
    template<typename T>
    T& getComponent(Entity entity) 
    {
        return m_registry.get<T>(entity);
    }
    
    template<typename T>
    bool hasComponent(Entity entity) const 
    {
        return m_registry.all_of<T>(entity);
    }
    
    template<typename T>
    void removeComponent(Entity entity) 
    {
        m_registry.remove<T>(entity);
    }
    
    template<typename... Components>
    auto view() 
    {
        return m_registry.view<Components...>();
    }
    
    entt::registry& getRegistry() 
    {
        return m_registry;
    }
    
    const entt::registry& getRegistry() const 
    {
        return m_registry;
    }

private:
    entt::registry m_registry;
}; 