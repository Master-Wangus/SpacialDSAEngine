===========================================================
CSD3150/CS350 Geometry Toolbox - Project 3
===========================================================

UI USAGE INSTRUCTIONS:
---------------------
- Use the Assignment 3 ImGui popup to determine the BVH settings, as well as rendered levels.
- Changing any settings except the level will rebuild the BVH.

  KEY MAPPINGS:
-----------
- W: Move forward 
- S: Move backward
- A: Move left
- D: Move right
- C: Switch between FPS and Debug camera modes
- Mouse Hold RMB: Look around 
- Mouse Scroll: Zoom in/out 
- Mouse LMB: Drag entity
- ESC: Exit application

ASSUMPTIONS AND REQUIREMENTS:
-------------------------------
- Requires OpenGL 4.6 compatible GPU and drivers
- Requires GLFW, GLEW, GLM, EnTT libraries and ImGui (automatically downloaded via CMake)
- Shader files must be in the correct location (w.qua-project-3/shaders/)
- Assumed right-handed coordinate system
- Assumed counter-clockwise winding order for triangles
- AABB stored as center and half-extents for easier transformations 

BUGS
------
- If pressing C does not swap to the top-down debug camera, click on the scene outside of any ImGui Context Windows, then press C again.

COMPONENT EVALUATION:
===================

Scene Setup (25%)
-------------------
- Loaded 8 distinct meshes from the supplied dataset: `bunny.obj`, `rhino.obj`, `cup.obj`, `gun.obj`, `arm.obj`, `cat.obj`, `stuffed.obj`, and `cube.obj`.
- For every object the following spatial bounds are pre-computed on load:
  1. Axis-Aligned Bounding Box (AABB) stored as centre + half-extents.
  2. Bounding Sphere radius obtained via PCA principal axis & max projection.
  3. Oriented Bounding Box (OBB) eigen-vectors of the covariance matrix give the local frame, half-sizes are extents in that frame.
- Camera rigs:
  - First-Person Camera 
  - Top-Down Camera – also supports PiP overlay when FPS is active.
- ImGui controls allow runtime toggling of: visible BV type (None / AABB / Sphere / OBB), active camera, and BVH parameters (split rule, termination rule, rendered tree depth).

Bounding Volume Hierarchy (70%)
-----------------------------------
- Supports using:
 - AABB
 - Bounding Spheres
 - OBB
### Top-Down builder
- Supports 3 split heuristics selectable in ImGui:
  1. Median of BV centres (balanced, cheap).
  2. Median of BV extents (reduces parent volume).
  3. K-even (binary) equal object counts on the axis with largest extent.
- Split axis (X / Y / Z) is chosen to minimise total child volume.
- Termination conditions (toggleable):
  - Leaf when 1 object, ≤2 objects, or tree height = 2.

### Bottom-Up builder
- Start with a leaf for every object; iteratively merge the pair that minimises the chosen heuristic:
  1. Nearest-Neighbour (centroid distance).
  2. Minimum combined volume.
  3. Minimum combined surface area.
- Merging stops when a single root remains.

### Implementation notes
- Nodes are colour-coded by depth for visual inspection (Red → Orange → Yellow → … → Violet).

EXTRA CREDIT (10%)
-----------------
- OBBs are included as part of the BVH Build BVH With.

OBSERVATIONS:
----------
- Median-centre yields balanced trees with fast traversal but slightly larger parent volumes.
- Median-extent reduces volume overlap at the cost of occasional imbalance.
- K-even mirrors spatial grids are effective for clustered scenes but sensitive to outliers.
- Nearest-neighbour bottom-up preserves locality yet can inflate volumes if clusters are elongated.
- Volume vs Surface-Area heuristic: surface area generally produces the tightest BVH in this dataset.

FILE PATHS AND IMPLEMENTATION DETAILS:
-----------------------------------
Header Files (include/):
- Bvh.hpp - Core BVH implementation with top-down and bottom-up builders
- Buffer.hpp - OpenGL buffer wrapper for geometry data
- CameraSystem.hpp - FPS and top-down camera implementations
- Components.hpp - Entity component definitions
- CubeRenderer.hpp - Cube primitive rendering
- DemoScene.hpp - Scene setup and object management
- EventSystem.hpp - Event handling system
- Geometry.hpp - Geometric primitives and operations
- ImGuiManager.hpp - UI controls and settings
- InputSystem.hpp - Input handling and mapping
- IRenderable.hpp - Rendering interface
- Keybinds.hpp - Input key definitions
- Lighting.hpp - Lighting system
- MeshRenderer.hpp - Mesh rendering system
- pch.h - Precompiled headers
- PickingSystem.hpp - Object selection and manipulation
- Registry.hpp - Entity registry system
- RenderSystem.hpp - Main rendering pipeline
- ResourceSystem.hpp - Resource management
- Shader.hpp - Shader program management
- Shapes.hpp - Basic shape definitions
- SphereRenderer.hpp - Sphere primitive rendering
- Systems.hpp - System management
- Window.hpp - Window and OpenGL context

Source Files (src/):
- Bvh.cpp - BVH implementation
- Buffer.cpp - Buffer management
- CameraSystem.cpp - Camera controls
- Components.cpp - Component implementations
- CubeRenderer.cpp - Cube rendering
- DemoScene.cpp - Scene management
- EventSystem.cpp - Event system
- Geometry.cpp - Geometry operations
- ImGuiManager.cpp - UI implementation
- InputSystem.cpp - Input processing
- main.cpp - Application entry
- MeshRenderer.cpp - Mesh rendering
- PickingSystem.cpp - Object picking
- Registry.cpp - Entity management
- RenderSystem.cpp - Render pipeline
- ResourceSystem.cpp - Resource handling
- Shader.cpp - Shader management
- Shapes.cpp - Shape implementations
- SphereRenderer.cpp - Sphere rendering
- Systems.cpp - System coordination
- Window.cpp - Window management

Shader Files (shaders/):
- my-project-3.vert - Vertex shader for 3D object rendering
- my-project-3.frag - Fragment shader for directional lighting

Model Files (models/):
- bunny.obj - Stanford Bunny mesh (201KB)
- rhino.obj - Rhinoceros model (300KB)
- cup.obj - Drinking cup model (1.2MB)
- gun.obj - Weapon model (30KB)
- arm.obj - Articulated arm model (7.2MB)
- cat.obj - Cat character model (5.1MB)
- stuffed.obj - Stuffed toy model (2.8MB)
- cube.obj - Simple cube primitive (798B)

TEST PLATFORM DETAILS:
-------------------
- Windows 10
- Visual Studio 2022
- NVIDIA GeForce RTX 3070 Laptop
- OpenGL 4.6

WEEKLY TIME BREAKDOWN:
-------------------
- Week 1: 20 hours - Fixing Project 2 issues, introduced picking system and event system.
- Week 2: 15 hours - Finished implementation of picking and event system. Core BVH system foundations implemented.
- Week 3: 20 hours - Finish implementation of AABB, Bounding Spheres and OBB Top-down and Bottom-up. Clean up files.
Total: 55 hours


