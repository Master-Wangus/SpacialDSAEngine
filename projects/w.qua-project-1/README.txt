===========================================================
CSD3150/CS350 Geometry Toolbox - Project 1
===========================================================

UI USAGE INSTRUCTIONS:
---------------------
- WASD: Move the camera (W - forward, S - backward, A - left, D - right)
- Mouse Hold Right Click: Look around (yaw and pitch camera control)
- Mouse Hold Left Click on Object: Drag object around to test collision.
- Color of entities:
 - Red: Collision
 - Blue: Non-Collision
 - Yellow: Dragged

ASSUMPTIONS AND CRASH CONDITIONS:
-------------------------------
- Requires OpenGL 4.6 compatible GPU and drivers
- Requires GLFW, GLEW, GLM, EnTT libraries and ImGui (automatically downloaded via CMake)
- Shader files must be in the correct location (w.qua-project-1/shaders/)

COMPLETED PARTS:
--------------
1. Window class (Task 1) - Complete
   - All GLFW functionality encapsulated in Window class
   - Input handling, events, and window state management
   - Error handling for GLFW calls

2. Buffer class (Tasks 2 & 3) - Complete
   - Centralized VBO management with VAO creation and binding
   - Supports vertex attributes: Position, Color, Normal, and Texture Coordinates

3. ECS (Task 4) - Complete
   - Uses EnTT library for Entity Component System architecture
   - Components: Transform, Mesh, Material, AABB, BoundingSphere, DirectionalLight
   - Systems: Transform update, Rendering, Collision detection

4. Lighting (Task 5) - Buggy
   - Implemented directional light
   - Phong lighting model with ambient, diffuse, and specular components
   - Per-fragment lighting calculations

5. Camera (Task 6) - Complete
   - First-person camera implementation
   - WASD movement and mouse look controls
   - View and projection matrix generation

6. Geometry Toolbox Core - Complete
   - Point3D, Plane, Triangle, Ray, BoundingSphere, AABB primitives
   - All required intersection tests:
     - Sphere vs Sphere, AABB vs AABB, Sphere vs AABB, AABB vs Sphere
     - Point vs Sphere/AABB/Plane/Triangle
     - Ray vs Plane/AABB/Sphere/Triangle
     - Plane vs AABB/Sphere

INCOMPLETE/BUGGY PARTS:
---------------------
- Triangle vs Point/Ray is buggy due to the rendering of Triangle is slightly off, resulting in having to drag the Triangle off its rendered shape around to test.
  The collider is correct but the render is offset by a bit.
- The same issue is occuring for Point
- Lighting is buggy, weird shadows

FILE PATHS, FUNCTION NAMES, AND LINE NUMBERS:
------------------------------------------
- Window Class: include/Window.hpp, src/Window.cpp
- Buffer Class: include/Buffer.hpp, src/Buffer.cpp
- Primitives: include/Primitives.hpp, src/Primitives.cpp
- Intersection Tests: include/Intersection.hpp, src/Intersection.cpp
- Camera Implementation: include/Camera.hpp, src/Camera.cpp
- Lighting: include/Lighting.hpp, src/Lighting.cpp
- ECS Components: include/Components.hpp, src/Components.cpp
- ECS Systems: include/Systems.hpp, src/Systems.cpp
- Main Application: src/main.cpp

TEST PLATFORM DETAILS:
-------------------
- Windows 10
- Visual Studio 2022
- NVIDIA GeForce RTX 3070
- OpenGL 4.6

WEEKLY TIME BREAKDOWN:
-------------------
- Week 1: 10 hours - Research, planning, and initial implementation
- Week 2: 15 hours - Core geometry primitives and intersection tests
- Week 3: 20 hours - ECS, lighting, camera, and final integration

KEY MAPPINGS:
-----------
- W: Move forward
- S: Move backward
- A: Move left
- D: Move right
- Mouse RMB: Look around
- Mouse LMB: Drag entity
- ESC: Exit application

ASSUMPTIONS:
----------
- Assumed right-handed coordinate system
- Assumed counter-clockwise winding order for triangles
- AABB stored as center and half-extents for easier transformations

KNOWN ISSUES:
-----------
- Sometimes the dragging of entities may stop or bug out. Move around and try again.
- Camera movement speed doesn't account for low frame rates 