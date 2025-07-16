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
  PRIVATE Eigen3::Eigen
)

# Include path for GLM (optional if not using FetchContent)
include_directories(${CMAKE_CURRENT_LIST_DIR}/lib/glm)

# ======================= AUTO PROJECT DISCOVERY =========================
enable_testing()

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
        target_link_libraries(${child} PRIVATE
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

        # ======================= TEST CONFIGURATION =========================
        # Check if tests directory exists
        if(IS_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/projects/${child}/tests")
            message(STATUS "Configuring tests for project: ${child}")
            
            # Collect test sources
            file(GLOB_RECURSE ${child}_test_files
                ${CMAKE_CURRENT_LIST_DIR}/projects/${child}/tests/*.cpp
                ${CMAKE_CURRENT_LIST_DIR}/projects/${child}/tests/*.hpp
            )
            
            # Get project sources excluding main.cpp for tests
            set(${child}_test_source_files ${${child}_source_files})
            list(FILTER ${child}_test_source_files EXCLUDE REGEX ".*main\\.cpp$")
            
            if(${child}_test_files)
                # Create test executable
                add_executable(${child}_tests
                    ${${child}_test_files}
                    ${${child}_test_source_files}
                )
                
                # Link with gtest and dependencies
                target_link_libraries(${child}_tests PRIVATE
                    gtest
                    gtest_main
                    ${ALL_LIBS}
                )
                
                set_property(TARGET ${child}_tests PROPERTY CXX_STANDARD 20)
                
                target_include_directories(${child}_tests PRIVATE
                    ${CMAKE_CURRENT_LIST_DIR}/projects/${child}/include
                    ${CMAKE_CURRENT_LIST_DIR}/projects/${child}/tests
                )
                
                # Add precompiled header for tests
                if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/projects/${child}/include/pch.h")
                    target_precompile_headers(${child}_tests PRIVATE
                        "${CMAKE_CURRENT_LIST_DIR}/projects/${child}/include/pch.h"
                    )
                endif()
                
                # Add test to CTest
                add_test(NAME ${child}_tests COMMAND ${child}_tests)
                
                # Store test target for later dependency setup
                set_property(GLOBAL APPEND PROPERTY TEST_TARGETS ${child}_tests)
                
                message(STATUS "Tests configured for project: ${child}")
            endif()
        endif()
        # ======================= END TEST CONFIGURATION ===================
    endif()
endforeach()

# ======================= END AUTO CONFIGURATION =========================

# ======================= GLOBAL TEST TARGETS =========================

# Custom target to run all tests (renamed to avoid VS conflicts)
add_custom_target(run_all_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure --verbose
    COMMENT "Running all tests..."
    USES_TERMINAL
)

# Add dependencies to run_all_tests for all test targets
get_property(test_targets GLOBAL PROPERTY TEST_TARGETS)
if(test_targets)
    add_dependencies(run_all_tests ${test_targets})
    list(LENGTH test_targets num_tests)
    message(STATUS "Added 'run_all_tests' target with ${num_tests} test dependencies.")
else()
    message(STATUS "Added 'run_all_tests' target. No tests found.")
endif()

message(STATUS "Use 'cmake --build . --target run_all_tests' to run all tests.")
