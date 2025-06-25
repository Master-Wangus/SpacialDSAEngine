/**
 * @class EventSystem
 * @brief System for managing events and observer patterns.
 *
 * This system handles event subscription, unsubscription and dispatching events 
 * to registered listeners across the application.
 */

#pragma once
#include "pch.h"

// Forward declarations
class Registry;
class Window;

// Event types enumeration
enum class EventType : uint16_t
{
    None = 0,
    
    // Window and input events
    KeyPress,
    KeyRelease,
    MouseButtonPress,
    MouseButtonRelease,
    MouseMove,
    MouseScroll,
    
    // Transform/scene updates
    TransformChanged,
    
    // Scene events
    SceneReset
};

// Variant of possible data types
using EventData = std::variant<
    std::monostate,       // Empty event data
    int,                  // For key codes, mouse buttons
    float,                // For scalar values
    bool,                 // For toggle states
    std::string,          // For messages, names, paths
    glm::vec2,            // For 2D positions, scroll deltas
    glm::vec3,            // For 3D positions, scales, etc.
    glm::vec4,            // For colors, quaternions
    glm::mat4,            // For transformations
    std::pair<int, int>,  // For window dimensions
    entt::entity          // For entity references
>;

using EventFunction = std::function<void(const EventData&)>;

class EventSystem; // Forward declaration for macros


class EventSystem
{
public:
    /**
     * @brief Gets the singleton instance of the EventSystem.
     * @return Reference to the EventSystem singleton
     */
    static EventSystem& Get();
    
    /**
     * @brief Initializes the event system.
     */
    void Initialize();
    
    /**
     * @brief Updates the event system each frame.
     * @param deltaTime Time elapsed since last frame
     */
    void Update(float deltaTime);
    
    /**
     * @brief Shuts down the event system and clears all subscriptions.
     */
    void Shutdown();
    
    /**
     * @brief Subscribe a function to an event type.
     * @param eventType The type of event to subscribe to
     * @param eventFunction The function to call when the event is fired
     */
    void SubscribeToEvent(const EventType eventType, EventFunction&& eventFunction);
    
    /**
     * @brief Fire an event of the specified type.
     * @param eventType The type of event to fire
     * @param eventData Optional data to send with the event
     */
    void FireEvent(const EventType eventType, EventData eventData = std::monostate());
    
    /**
     * @brief Get the number of observers for a specific event type.
     * @param eventType The event type to query
     * @return The number of observers
     */
    size_t GetObserverCount(EventType eventType) const;
    
    /**
     * @brief Clear all subscriptions for a specific event type.
     * @param eventType The event type to clear subscriptions for
     */
    void ClearEventSubscriptions(EventType eventType);
    
    /**
     * @brief Clear all event subscriptions.
     */
    void ClearAllEventSubscriptions();
    
private:
    /**
     * @brief Private constructor to enforce singleton pattern.
     */
    EventSystem();
    
    /**
     * @brief Destructor cleans up event subscriptions.
     */
    ~EventSystem();
    
    // Delete copy and move constructors/operators
    EventSystem(const EventSystem&) = delete;
    EventSystem& operator=(const EventSystem&) = delete;
    EventSystem(EventSystem&&) = delete;
    EventSystem& operator=(EventSystem&&) = delete;
    
    std::unordered_map<EventType, std::vector<EventFunction>> m_EventMap;
    bool m_Initialized = false;
}; 