===========================================================
CSD3150/CS350 Spatial Partitioning - Project 4
===========================================================

UI USAGE INSTRUCTIONS
-----------------------------------------------------------
- Use the Assignment 4 ImGui popup to expose all run-time controls:
1. Octree parameters  – max objects / max depth / straddling rule.
2. KD-Tree parameters – max objects / max depth / split rule.
3. Render toggles     – wire-frame, bounding volumes, tree cell visibility, level colouring, frustum culling.
Any change except the *Rendered Level* slider forces an automatic rebuild of the
corresponding tree.

KEY MAPPINGS
-----------------------------------------------------------
1. W A S D      – first-person translation (forward / left / back / right)
2. Mouse  RMB   – freelook / rotate camera while held
3. Mouse  LMB   – select & drag nearest entity
4. Mouse  Wheel – zoom in / out (orbital camera)
5. C            – toggle between FPS and Top-Down debug camera
6. R            – reset the entire scene
7. F            – toggle global wire-frame mode
8. SHIFT (hold) – increase camera movement speed
9. ESC          – quit application

ADAPTIVE OCTREE
-----------------------------------------------------------
1. Straddling Resolution Methods (`enum StraddlingMethod`, see `include/Octree.hpp`):
   1.1 UseCenter          – place object in the child that contains its centre point.
   1.2 StayAtCurrentLevel – keep objects that overlap several children in the current node (avoids duplication).
2. Termination Criteria (implemented in `src/Octree.cpp` `BuildOctree()`, lines 74-88):
   – depth ≥ `m_MaxDepth` OR object count ≤ `m_MaxObjects` OR no objects.
3. Coloured Level Rendering: each node is visualised with a `CubeRenderer` using the hue table in `SpatialTreeUtils::LevelColor()`.

KD-TREE
-----------------------------------------------------------
1. Split Methods (`enum KdSplitMethod`, see `include/KDTree.hpp`):
   1.1 MedianCenter  – median of object centres on current axis.
   1.2 MedianExtent  – median of object half-extents on current axis.
2. Termination Criteria (implemented in `src/KDTree.cpp` `BuildKdTree()`, lines 39-55):
   – depth ≥ `m_MaxDepth` OR object count ≤ `m_MaxObjects` OR empty set; additionally, if either side of a split is empty the node is forced leaf.
3. Level colouring identical to the octree.

COMPONENT EVALUATION:
===================

Scene Creation (15%)
-------------------
- Loaded 8 distinct meshes from the supplied dataset (see DemoScene.cpp).

Adaptive Octree (40%)
-------------------
- Creation of adaptive octree.
- Two straddling resolution methods (UseCenter / StayAtCurrentLevel).
- Termination criteria (maxDepth / maxObjects).
- Colored level rendering.

KD-Tree (40%)
-------------------
- Creation of KD-tree.
- Two split-point strategies (MedianCenter / MedianExtent).
- Termination criteria (maxDepth / maxObjects).
- Colored level rendering.

Miscellaneous Issues (5%)
-------------------
- README present.
- Application builds & runs without errors.

COMPLETED / INCOMPLETE FEATURES
-----------------------------------------------------------
Completed:
1. Entire scene & asset loading (8 obj + UNC sections).
2. Fully functional adaptive octree & KD-tree with ImGui live editing.
3. Frustum culling (toggle) and object picking / dragging.
4. Colour-coded cell visualisation, wire-frame toggle, bounding volume options.

FILE PATHS AND IMPLEMENTATION DETAILS:
-----------------------------------
Header Files (include/):
- Buffer.hpp - OpenGL VBO / VAO wrapper and helper utilities
- CameraSystem.hpp - First-person & orbital cameras plus frustum culling maths
- Components.hpp - Definitions for ECS components (transform, render, camera, light, BV, etc.)
- CubeRenderer.hpp - Wire-frame cube visualisation (tree cells, AABBs, OBBs)
- DemoScene.hpp - Scene creation helpers & section scaling API
- EventSystem.hpp - Global pub / sub event bus
- Geometry.hpp - Low-level geometry helpers (plane tests, AABB transform)
- IRenderable.hpp - Abstract base interface for anything that can be drawn
- ImGuiManager.hpp - Dear ImGui initialisation and debug UI panels
- InputSystem.hpp - Keyboard / mouse state tracking & callbacks
- KDTree.hpp - KD-tree node structure and public API
- Keybinds.hpp - Centralised key-code and mouse-button constants
- Lighting.hpp - Directional light + material structs for UBOs
- MeshRenderer.hpp - Draws mesh resources with per-object material
- Octree.hpp - Adaptive octree node structure and public API
- PickingSystem.hpp - Ray-cast picking & drag-move implementation
- Registry.hpp - Wrapper around EnTT registry with helper functions
- RenderSystem.hpp - Main rendering pipeline (lights, materials, BV toggles)
- ResourceSystem.hpp - Mesh loading / caching via Assimp
- Shader.hpp - GLSL program compilation helper
- Shapes.hpp - Basic volume structs (Aabb, Sphere, Obb)
- SpatialTreeUtils.hpp - Helper math for tree building & level colours
- SphereRenderer.hpp - Wire-frame sphere visualisation (BVH spheres)
- Systems.hpp - Global pointers, init / shutdown of all systems
- Window.hpp - GLFW window wrapper + input callback glue
- pch.h - Pre-compiled headers for common libs

Source Files (src/):
- Buffer.cpp - Implements Buffer.hpp
- CameraSystem.cpp - Camera movement & frustum extraction
- Components.cpp - Helper functions for complex components
- CubeRenderer.cpp - Cube mesh generation & draw
- DemoScene.cpp - Loads OBJ meshes, UNC power-plant, sets transform scales
- EventSystem.cpp - EventSystem singleton implementation
- Geometry.cpp - Plane / frustum / BV tests & maths routines
- ImGuiManager.cpp - Renders all ImGui windows incl. Assignment-4 panel
- InputSystem.cpp - Polls / stores keyboard & mouse state
- KDTree.cpp - Recursive KD-tree builder & visualiser
- Octree.cpp - Recursive adaptive octree builder & visualiser
- PickingSystem.cpp - Mouse-ray intersection tests and drag plane logic
- RenderSystem.cpp - Master renderer (calls BuildOctree / BuildKDTree)
- ResourceSystem.cpp - Loads OBJ via Assimp into MeshResource cache
- Shapes.cpp - Constructors & helper methods for Aabb / Sphere / Obb
- Shader.cpp - GL shader compile / link / uniform cache
- SphereRenderer.cpp - Generates UV-sphere vertices and draw call
- Systems.cpp - Initialises global systems & update loop glue
- Window.cpp - GLFW window management and callback dispatch
- main.cpp - Application entry point & main loop
- Registry.cpp - Thin wrappers for create / destroy entity

Unit Tests (tests/):
- TestGeometry.cpp - Validates plane / frustum classification helpers
- TestKDTree.cpp - Ensures KD-tree splits & termination behave correctly
- TestOctree.cpp - Ensures adaptive octree splits & straddle logic
- TestShapes.cpp - Tests basic Aabb, Sphere maths operations

Shader Files (shaders/):
- my-project-4.vert – vertex shader for mesh & tree visualisation.
- my-project-4.frag – fragment shader with single directional light.

Model Assets (models/):
- bunny.obj, rhino.obj, cup.obj, gun.obj, cube.obj, arm.obj, cat.obj, stuffed.obj.
- UNC power-plant sections stored in models/unc/ (text lists consumed at runtime).

Implementation Highlights:
- Adaptive octree core: `src/Octree.cpp` (functions `BuildOctree`, `DistributeObjectsToChildren`).
- KD-tree core:        `src/KDTree.cpp` (functions `ChooseSplitPosition`, `BuildKdTree`).
- Rendering hookup:    `src/RenderSystem.cpp` (`BuildOctree` lines 240-280, `BuildKDTree` lines 300-340).
- Runtime controls:    `src/ImGuiManager.cpp` (`RenderAssignment4Controls` lines 210-320).

TEST PLATFORM DETAILS:
-------------------
- Windows 10
- Visual Studio 2022
- NVIDIA GeForce RTX 3070 Laptop
- OpenGL 4.6

WEEKLY TIME BREAKDOWN:
-------------------
- Week 1: 20 hours - Implementing scene loading of big meshes and octtrees.
- Week 2: 15 hours - Fix octtree bugs and implemented kd-trees.
- Week 3: 10 hours - Test system implementation and documentation.
Total: 45 hours

ASSUMPTIONS & KNOWN ISSUES
-----------------------------------------------------------
1. Right-handed coordinate system, CCW winding.
2. GPU must support OpenGL 4.6 core profile.
3. Shader files are expected in `projects/w.qua-project-4/shaders/`.
4. If 'C' (camera toggle) seems unresponsive, ensure the mouse is over the viewport (not an ImGui window) before pressing the key.
5. Huge meshes (> 2 M triangles) will cause a noticeable rebuild pause (single thread).
6. Limiting the depth of the tree is necessary to prevent infinite subdivision.

OBSERVATIONS / NOTES
-----------------------------------------------------------
1. The StayAtCurrentLevel straddling rule produces taller but narrower octrees; useful when the scene has many elongated objects.
2. Median-extent splitting in KD-tree yields fewer SAH misses for our dataset than median-centre; however it can create unbalanced trees for clustered geometry.
3. Both data-structures are rebuilt from scratch each frame only when the user changes parameters – current test scene rebuilds in ≤ 1 ms on the 3070 GPU.



