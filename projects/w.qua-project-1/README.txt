===========================================================
CSD3150/CS350 Geometry Toolbox - Project 1
===========================================================

UI USAGE INSTRUCTIONS:
---------------------
- Use the Geometry Toolbox Controls ImGui popup to control select the testscenes.
- You can also drag the light source (represented by the yellow sphere at the top).
- WASD: Move the camera (FPS mode) or move target point (Orbital mode)
- Mouse Hold Right Click: Look around (FPS mode) or orbit around target (Orbital mode)
- Mouse Scroll Wheel: Zoom in/out (Orbital mode only)
- C Key: Switch between FPS and Orbital camera modes
- Mouse Hold Left Click on Object: Drag object around to test collision.
- Color of entities:
  - Red: Collision
  - Green and Blue: Non-Collision
  - Yellow: Dragged

  KEY MAPPINGS:
-----------
- W: Move forward (FPS) / Move target forward (Orbital)
- S: Move backward (FPS) / Move target backward (Orbital)
- A: Move left (FPS) / Move target left (Orbital)
- D: Move right (FPS) / Move target right (Orbital)
- C: Switch between FPS and Orbital camera modes
- Q/E: Rotate ray (in ray-based demos)
- Mouse RMB: Look around (FPS) / Orbit around target (Orbital)
- Mouse Scroll: Zoom in/out (Orbital mode only)
- Mouse LMB: Drag entity
- ESC: Exit application

ASSUMPTIONS AND REQUIREMENTS:
-------------------------------
- Requires OpenGL 4.6 compatible GPU and drivers
- Requires GLFW, GLEW, GLM, EnTT libraries and ImGui (automatically downloaded via CMake)
- Shader files must be in the correct location (w.qua-project-1/shaders/)

COMPONENT EVALUATION:
------------------
Task Components (25%):
1. Window class (5%) - Complete
   - All GLFW functionality encapsulated in Window class
   - Input handling, events, and window state management
   - Error handling for GLFW calls

2. Buffer class (5%) - Complete
   - Centralized VBO management with VAO creation and binding
   - Supports vertex attributes: Position, Color, Normal, and Texture Coordinates

3. ECS (5%) - Complete
   - Uses EnTT library for Entity Component System architecture
   - Components: Transform, Mesh, Material, AABB, BoundingSphere, DirectionalLight
   - Systems: Transform update, Rendering, Collision detection

4. Lighting (5%) - Complete
   - Implemented directional light
   - Phong lighting model with ambient, diffuse, and specular components
   - Per-fragment lighting calculations

5. Camera (5%) - Complete
   - Dual camera system: First-person (FPS) and Orbital camera modes
   - FPS mode: WASD movement and mouse look controls
   - Orbital mode: Mouse orbit around target, scroll wheel zoom, WASD target movement
   - Seamless switching between modes with 'C' key
   - View and projection matrix generation for both camera types

Collision Tests (70%) - Complete
- Point3D, Plane, Triangle, Ray, BoundingSphere, AABB primitives
- All required intersection tests:
  - Sphere vs Sphere, AABB vs AABB, Sphere vs AABB, AABB vs Sphere
  - Point vs Sphere/AABB/Plane/Triangle
  - Ray vs Plane/AABB/Sphere/Triangle
  - Plane vs AABB/Sphere

EXTRA CREDIT ACHIEVEMENTS (30%):
------------------------------
High-Quality Implementation (+20%):
- All tasks completed on time
- Thoughtful design with clean, well-structured code architecture
- Comprehensive error handling and edge case management
- Detailed documentation and intuitive user interface
- Attention to detail in all system components

Orbital Camera System (+10%):
- Implemented orbital camera with full 3D rotation around target point
- Mouse-controlled orbital movement with intuitive controls
- Scroll wheel zoom with configurable min/max distance constraints
- WASD target point movement for dynamic scene exploration
- Seamless camera mode switching with 'C' key
- Real-time camera state display in ImGui interface
- Smooth transitions between FPS and Orbital modes
- Comprehensive camera controls documentation in UI

FILE PATHS AND IMPLEMENTATION DETAILS:
-----------------------------------
Header Files (include/):
- Buffer.hpp - OpenGL buffer wrapper for 3D geometry data management
- CollisionSystem.hpp - System for detecting and handling collisions
- Components.hpp - Component definitions for entity-component system
- CubeRenderer.hpp - Renderer for 3D cube primitives
- DemoScene.hpp - Demo scene with multiple 3D objects and interaction
- CameraSystem.hpp - System for dual camera modes (FPS and Orbital) with seamless switching
- ImGuiManager.hpp - Manager for Dear ImGui integration and UI
- InputSystem.hpp - System for handling user input
- Intersection.hpp - 3D geometric collision detection algorithms
- IRenderable.hpp - Interface for renderable 3D objects
- Keybinds.hpp - Constants for keyboard and mouse input handling
- Lighting.hpp - Definitions for lighting and material properties
- ObjectManipulationSystem.hpp - System for interactive object manipulation
- pch.h - Precompiled header with common dependencies
- PlaneRenderer.hpp - Renderer for 3D plane primitives
- Primitives.hpp - Definitions of 3D geometric primitive shapes
- RayRenderer.hpp - Renderer for 3D ray primitives
- Registry.hpp - Central registry for entity-component management
- RenderSystem.hpp - System for rendering 3D objects and scenes
- Shader.hpp - GLSL shader program manager
- SphereRenderer.hpp - Renderer for 3D sphere primitives
- Systems.hpp - System management and coordination
- TriangleRenderer.hpp - Renderer for 3D triangle primitives
- Window.hpp - OpenGL rendering window and application context

Source Files (src/):
- Buffer.cpp - Implementation of OpenGL buffer wrapper
- CollisionSystem.cpp - Implementation of collision detection system
- Components.cpp - Implementation of component functionality
- CubeRenderer.cpp - Implementation of cube rendering
- DemoScene.cpp - Implementation of demo scene setup
- CameraSystem.cpp - Implementation of dual camera system (FPS and Orbital modes)
- ImGuiManager.cpp - Implementation of ImGui integration
- InputSystem.cpp - Implementation of input handling
- Intersection.cpp - Implementation of collision detection algorithms
- main.cpp - Entry point for the application
- ObjectManipulationSystem.cpp - Implementation of object manipulation
- PlaneRenderer.cpp - Implementation of plane rendering
- Primitives.cpp - Implementation of 3D primitives
- RayRenderer.cpp - Implementation of ray rendering
- Registry.cpp - Implementation of entity registry
- RenderSystem.cpp - Implementation of rendering system
- Shader.cpp - Implementation of shader program management
- SphereRenderer.cpp - Implementation of sphere rendering
- Systems.cpp - Implementation of system management
- TriangleRenderer.cpp - Implementation of triangle rendering
- Window.cpp - Implementation of window management

Shader Files (shaders/):
- my-project-1.vert - Vertex shader for 3D object rendering
- my-project-1.frag - Fragment shader for directional lighting

TEST PLATFORM DETAILS:
-------------------
- Windows 10
- Visual Studio 2022
- NVIDIA GeForce RTX 3070 Laptop
- OpenGL 4.6

WEEKLY TIME BREAKDOWN:
-------------------
- Week 1: 10 hours - Planning, and initial implementation
- Week 2: 15 hours - Core geometry primitives and intersection tests
- Week 3: 20 hours - ECS, lighting, camera, and final integration
Total: 45 hours

ASSUMPTIONS:
----------
- Assumed right-handed coordinate system
- Assumed counter-clockwise winding order for triangles
- AABB stored as center and half-extents for easier transformations 
- File header is documented. Function names and parameters are explicitly written over function
  documentation and it is easier to read