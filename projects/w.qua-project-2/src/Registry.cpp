/**
 * @class Registry
 * @brief Central registry for entity-component management.
 *
 * This class implements a centralized registry for entity-component system (ECS) architecture,
 * handling entity creation, component attachment, and system registration.
 */

#include "Registry.hpp"

Registry::Entity Registry::Create() 
{
    return m_Registry.create();
}

void Registry::Destroy(Entity entity) 
{
    m_Registry.destroy(entity);
} 