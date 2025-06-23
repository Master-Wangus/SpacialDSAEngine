/**
 * @class DemoScene
 * @brief Demonstration scene for rendering 3D mesh objects.
 *
 * This class sets up a 3D scene with mesh objects and lighting to showcase
 * the engine's mesh rendering capabilities.
 */

#pragma once

#include "pch.h"

class Window;
class Registry;

enum class DemoSceneType
{
    MeshScene  
};

enum class ModelType
{
    Rhino,
    Cup,
    Bunny,
    Cube,
    Gun,
    Cat,
    Arm,
    Stuffed,
    Count  
};

namespace DemoScene
{
    /**
     * @brief Sets up the demonstration scene with the specified type.
     * @param registry Entity registry to populate with scene objects
     * @param window Window reference for context
     * @param sceneType Type of scene to set up
     */
    void SetupScene(Registry& registry, Window& window, DemoSceneType sceneType = DemoSceneType::MeshScene);
    
    /**
     * @brief Clears all objects from the scene.
     * @param registry Entity registry to clear
     */
    void ClearScene(Registry& registry);
    
    /**
     * @brief Resets the scene to its initial state.
     * @param registry Entity registry to reset
     * @param window Window reference for context
     */
    void ResetScene(Registry& registry, Window& window);
    
    /**
     * @brief Sets up a scene specifically for mesh rendering demonstration.
     * @param registry Entity registry to populate with mesh objects
     */
    void SetupMeshScene(Registry& registry);
    
    /**
     * @brief Switches the scene to display a specific model type.
     * @param registry Entity registry to modify
     * @param modelType Type of model to display
     */
    void SwitchToModel(Registry& registry, ModelType modelType);
    
    /**
     * @brief Gets the currently displayed model type.
     * @return Current model type being displayed
     */
    ModelType GetCurrentModel();
    
    /**
     * @brief Cycles to the next model in the sequence.
     * @param registry Entity registry to modify
     */
    void CycleToNextModel(Registry& registry);
    
    /**
     * @brief Cycles to the previous model in the sequence.
     * @param registry Entity registry to modify
     */
    void CycleToPreviousModel(Registry& registry);

    /**
     * @brief Sets a uniform scale for all models in the scene.
     * @param registry Entity registry reference
     * @param scale Uniform scale factor to apply (uniform across X,Y,Z)
     */
    void SetGlobalScale(Registry& registry, float scale);

    /**
     * @brief Sets the scale for a specific model type.
     * @param registry Entity registry reference
     * @param modelType Model whose scale should be changed
     * @param scale Uniform scale factor to apply
     */
    void SetModelScale(Registry& registry, ModelType modelType, float scale);

    /**
     * @brief Gets the current scale (uniform) for a model type.
     * @param registry Entity registry reference
     * @param modelType Model to query
     * @return The uniform scale value (assumes X scale represents uniform scale)
     */
    float GetModelScale(Registry& registry, ModelType modelType);
} 