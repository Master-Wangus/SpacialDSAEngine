#include "../include/Registry.hpp"

Registry::Entity Registry::Create() 
{
    return m_Registry.create();
}

void Registry::Destroy(Entity entity) 
{
    m_Registry.destroy(entity);
} 