===========================================================
CSD3150/CS350 Geometry Toolbox - Project 3
===========================================================

UI USAGE INSTRUCTIONS:
---------------------
- Use the Assignemnt 2 ImGui popup to select the models and bounding volumes.
- You can also control the FOV to determine culling checks.
- WASD: Move the camera (FPS mode) or move target point (Orbital mode)
- Mouse Hold Right Click: Look around (FPS mode) or orbit around target (Orbital mode)
- Mouse Scroll Wheel: Zoom in/out (Orbital mode only)
- C Key: Switch between FPS and Orbital camera modes
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
- R: Reset scene to initial state
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
- Assumed right-handed coordinate system
- Assumed counter-clockwise winding order for triangles
- AABB stored as center and half-extents for easier transformations 

BUGS
------
- The rendering of frustrum detection with bounding volumes is buggy:
 + It will display red for Aabbs even though the edge of the bounds of the fustrum is clearly intersecting and likewise for intersection,
   even when the Aabb is inside the fustrum it still renders yellow instead of green. The same can be observed for spheres however this is more
   of a rendering issue because the actual fustrum is actually offset to the right. This can get worse when done in fullscreen. I am unable to figure out
   the cause of the issue for rendering (I suspect its because of an aspect ratio issue). 
- The lines of the rendered fustrum will flicker, this is a floating point issue that I tried to resolve in FustrumRenderer::UpdateFrustum dosent seem to work...

COMPONENT EVALUATION:
===================

🧱 Scene Setup (10%)
-------------------
✅ Load a scene with at least 4 objects (e.g. bunny, cube, cup, rhino)
   - Implementation: DemoScene.hpp/cpp - Manages multiple 3D models
   - Uses Assimp library integration through ResourceSystem for .obj file loading
   - Supports dynamic model switching and scene management

✅ Use Assimp (or similar library) to load .obj files (ignore .mtl)
   - Implementation: ResourceSystem.hpp/cpp - Asset loading and management system
   - Mesh loading functionality in MeshRenderer.hpp/cpp
   - Models stored in models/ directory, loaded at runtime

✅ Implement:
   🔸 A First-Person Camera (WASD)
     - Implementation: CameraSystem.hpp/cpp - FPS camera with WASD movement
     - Mouse look controls with right-click and hold
     - Smooth movement and rotation handling
   
   🔸 A Top-Down Debug Camera Or Orbital Camera
     - Implementation: CameraSystem.hpp/cpp - Orbital camera system
     - Mouse orbit controls around target point
     - Scroll wheel zoom with distance constraints
     - WASD target point movement for scene exploration

📦 Bounding Volume Calculations (60%)
-----------------------------------
✅ Compute and implement:
   🔸 AABB (Axis-Aligned Bounding Box)
     - Implementation: Geometry.hpp/cpp - AABB calculation functions
     - Components.hpp - Bounding component for entities
     - Efficient min/max vertex computation for mesh data
   
   🔸 Bounding Spheres (BSphere) using:
     • Ritter's Method
       - Implementation: Geometry.hpp/cpp - CreateSphereRitters()
       - Finds initial sphere, then expands to encompass all points
       - Fast algorithm with good average-case performance
     
     • Modified Larsson's Method
       - Implementation: Geometry.hpp/cpp - CreateSphereIterative()
       - Iterative improvement of sphere bounds
       - Better sphere fit than basic methods
     
     • PCA-based Method (using covariance and eigen vectors)
       - Implementation: Geometry.hpp/cpp - CreateSpherePCA()
       - Uses Principal Component Analysis for optimal sphere placement
       - Computes covariance matrix and eigenvectors for best fit
   
   🔸 OBB (Oriented Bounding Box) using PCA
     - Implementation: Geometry.hpp/cpp - CreateObbPCA()
     - Uses Principal Component Analysis to find optimal orientation
     - Computes eigenvectors as OBB axes for minimal volume enclosure

🔍 Bounding Volume Display (25%)
-------------------------------
✅ Wireframe rendering of bounding volumes
   - Implementation: CubeRenderer.hpp/cpp - AABB/OBB wireframe rendering
   - SphereRenderer.hpp/cpp - Bounding sphere wireframe rendering
   - Dedicated wireframe rendering modes for all volume types

✅ Toggle visibility of actual object (wireframe or hidden)
   - Implementation: RenderSystem.hpp/cpp - Visibility toggle controls
   - ImGuiManager.hpp/cpp - UI controls for object visibility
   - Support for wireframe, solid, and hidden rendering modes

✅ Allow switching between volume types
   - Implementation: ImGuiManager.hpp/cpp - Bounding volume type selection
   - Runtime switching between AABB, BSphere (all methods), and OBB
   - Dynamic bounding volume computation and display

✅ Use distinct colors for:
   🔸 Inside frustum - GREEN
   🔸 Outside frustum - RED
   🔸 Intersecting frustum - YELLOW
   - Implementation: Geometry.hpp/cpp - Frustum intersection testing
   - RenderSystem.hpp/cpp - Color-coded rendering based on frustum state


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
- my-project-2.vert - Vertex shader for 3D object rendering
- my-project-2.frag - Fragment shader for directional lighting

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

BVH IMPLEMENTATION (Project 3 Extension)
=======================================

Bounding Volume Hierarchy (BVH)
-------------------------------
• Two construction modes: Top-Down and Bottom-Up.

Top-Down
~~~~~~~~
1. Objects are recursively split into two child nodes until a termination
   condition is met.
2. Supported split heuristics:
   • Median of BV centres (default)
   • Median of BV extents
   • K-even split (k = 2)
3. Supported termination conditions:
   • Leaf contains a single object
   • Leaf contains a maximum of two objects
   • Tree height equals two

Bottom-Up
~~~~~~~~~
1. Start with a leaf node for every object.
2. Repeatedly merge the pair of nodes that minimises a user-selected cost:
   • Centre-to-centre distance (Nearest-Neighbour)
   • Combined volume
   • Combined surface area

Usage
~~~~~
The ImGui "Assignment 2" window now has a "BVH Construction & Visualization"
section where the build method, split/merge heuristic, and termination rule
can be selected. Press "Build BVH" to rebuild the hierarchy. Use the "Show BVH"
checkbox to toggle rendering.

Visualisation
~~~~~~~~~~~~~
Every BVH level is rendered in a unique colour (red, orange, yellow, green,
blue, indigo, violet) cycling for deeper levels. Both AABB (cube wireframe) and
sphere visualisation are supported.

Observations
~~~~~~~~~~~~
• Median-centre produces balanced binary trees with good traversal
  performance but occasionally higher node volume.
• Median-extent often reduces parent volume but may create unbalanced trees and
  slightly deeper hierarchies.
• K-even (binary) mimics spatial grids; useful for very clustered scenes but
  sensitive to outliers.
• Nearest-neighbour bottom-up merges geometrically close objects first and tends
  to preserve locality but can inflate volume.
• Volume and surface-area heuristics both aim to minimise overlap; surface area
  typically yields slightly tighter bounds in practice.

