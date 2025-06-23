/**
 * @file pch.h
 * @brief Precompiled header with common dependencies for the project.
 *
 * This file contains standard library includes, third-party libraries, and common definitions
 * that are used throughout the project to improve compilation speed.
 */

#pragma once

// GLEW
#include <GL/glew.h>

// Standard library
#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <functional>
#include <unordered_map>
#include <map>
#include <algorithm>
#include <stdexcept>
#include <chrono>
#include <fstream>
#include <sstream>
#include <variant>

// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

// ImGui
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

// EnTT
#include <entt/entt.hpp> 

// Assimp
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

// Eigen
#include <Eigen/Dense>

// Engine includes
#include "Registry.hpp"
#include "EventSystem.hpp"