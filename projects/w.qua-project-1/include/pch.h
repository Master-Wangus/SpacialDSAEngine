/**
 * @file pch.h
 * @brief Precompiled header with common dependencies for the project.
 *
 * This file contains standard library includes, third-party libraries, and common definitions
 * that are used throughout the project to improve compilation speed.
 */

#pragma once

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

// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// ImGui
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

// EnTT
#include <entt/entt.hpp> 

// Engine includes
#include "Registry.hpp"