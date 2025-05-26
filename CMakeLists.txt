# ======================= Project Configuration =========================
cmake_minimum_required(VERSION 3.16...3.22)  # Increased minimum version for better PCH support

# Set the CMake policy version (this enables all policies up to CMake 3.5)
cmake_policy(VERSION 3.5)

project(csd-3151)

# Set the directory where custom CMake modules are stored
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

# Include project-wide utility and dependency modules
include(ImportDependencies)
importDependencies()

# Global compile flags and macros
add_definitions(-DGLEW_STATIC)
add_definitions(-DUSE_CSD3151_AUTOMATION=0) # Used for instructor's automation

# List of external libraries
set(ALL_LIBS
  PRIVATE glfw
  PRIVATE libglew_static
  EnTT::EnTT
  PRIVATE imgui
  PRIVATE assimp
)

# Include path for GLM (optional if not using FetchContent)
include_directories(${CMAKE_CURRENT_LIST_DIR}/lib/glm)

# ======================= AUTO PROJECT DISCOVERY =========================

# Get all subdirectories in the projects folder
file(GLOB children RELATIVE "${CMAKE_CURRENT_LIST_DIR}/projects" "${CMAKE_CURRENT_LIST_DIR}/projects/*")

foreach(child ${children})
    if(IS_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/projects/${child}")
        message(STATUS "Configuring project: ${child}")

        # Collect sources
        file(GLOB_RECURSE ${child}_source_files
            ${CMAKE_CURRENT_LIST_DIR}/projects/${child}/src/*.cpp
            ${CMAKE_CURRENT_LIST_DIR}/projects/${child}/include/*.h
            ${CMAKE_CURRENT_LIST_DIR}/projects/${child}/include/*.hpp
        )

        # Create executable
        add_executable(${child}
            ${${child}_source_files}
        )

        # Link libraries
        target_link_libraries(${child}
            ${ALL_LIBS}
        )

        # Set C++ standard
        set_property(TARGET ${child} PROPERTY CXX_STANDARD 20)

        # Warnings based on compiler
        if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
            target_compile_options(${child} PRIVATE -Wall ${DisableWarnings})
        elseif (MSVC)
            target_compile_options(${child} PRIVATE /W3 /WX-)
        endif()

        # Include path for each project
        target_include_directories(${child} PRIVATE
            ${CMAKE_CURRENT_LIST_DIR}/projects/${child}/include
        )
        
        # Add precompiled header
        if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/projects/${child}/include/pch.h")
            target_precompile_headers(${child} PRIVATE
                "${CMAKE_CURRENT_LIST_DIR}/projects/${child}/include/pch.h"
            )
        endif()
    endif()
endforeach()

# ======================= END AUTO CONFIGURATION =========================
