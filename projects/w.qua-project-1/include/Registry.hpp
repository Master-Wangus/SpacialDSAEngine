#pragma once

#include <entt/entt.hpp>
#include <memory>

class Registry
{
public:
    using Entity = entt::entity;
    
    Registry() = default;
    
    Entity Create();
    void Destroy(Entity entity);
    
    template<typename T, typename... Args>
    T& AddComponent(Entity entity, Args&&... args) 
    {
        return m_Registry.emplace<T>(entity, std::forward<Args>(args)...);
    }
    
    template<typename T>
    T& GetComponent(Entity entity) 
    {
        return m_Registry.get<T>(entity);
    }
    
    template<typename T>
    bool HasComponent(Entity entity) const 
    {
        return m_Registry.all_of<T>(entity);
    }
    
    template<typename T>
    void RemoveComponent(Entity entity) 
    {
        m_Registry.remove<T>(entity);
    }
    
    template<typename... Components>
    auto View() 
    {
        return m_Registry.view<Components...>();
    }
    
    entt::registry& GetRegistry() 
    {
        return m_Registry;
    }
    
    const entt::registry& GetRegistry() const 
    {
        return m_Registry;
    }

private:
    entt::registry m_Registry;
}; 