#include "../include/Registry.hpp"

Registry::Entity Registry::create() 
{
    return m_registry.create();
}

void Registry::destroy(Entity entity) 
{
    m_registry.destroy(entity);
} 