#include "pch.h"
#include "EventSystem.hpp"
#include "Registry.hpp"
#include "Window.hpp"

// Static singleton instance
static EventSystem* s_Instance = nullptr;

EventSystem& EventSystem::Get()
{
    if (!s_Instance)
    {
        s_Instance = new EventSystem();
    }
    return *s_Instance;
}

EventSystem::EventSystem()
    : m_Initialized(false)
{
}

EventSystem::~EventSystem()
{
    Shutdown();
}

void EventSystem::Initialize()
{
    if (m_Initialized) 
    {
        return;
    }
    
    // Clear any existing event subscriptions
    ClearAllEventSubscriptions();
    
    m_Initialized = true;
}

void EventSystem::Shutdown()
{
    if (!m_Initialized) 
    {
        return;
    }
    
    // Clear all event subscriptions
    ClearAllEventSubscriptions();
    
    m_Initialized = false;
}

void EventSystem::SubscribeToEvent(const EventType eventType, EventFunction&& eventFunction)
{
    m_EventMap[eventType].push_back(eventFunction);
}

void EventSystem::FireEvent(const EventType eventType, EventData eventData)
{
    // Check if the event type exists in the map
    auto it = m_EventMap.find(eventType);
    if (it == m_EventMap.end()) 
    {
        return; // No subscribers for this event type
    }
    
    const auto& subscribers = it->second;
    for (const auto& subscriber : subscribers) {
        subscriber(eventData);
    }
}

size_t EventSystem::GetObserverCount(EventType eventType) const
{
    auto it = m_EventMap.find(eventType);
    if (it == m_EventMap.end()) {
        return 0;
    }
    
    return it->second.size();
}

void EventSystem::ClearEventSubscriptions(EventType eventType)
{
    auto it = m_EventMap.find(eventType);
    if (it != m_EventMap.end()) {
        it->second.clear();
    }
}

void EventSystem::ClearAllEventSubscriptions()
{
    m_EventMap.clear();
} 