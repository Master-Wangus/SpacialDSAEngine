include(FetchContent)

# Macro to import GLFW
macro(import_glfw)
    if(NOT TARGET glfw)  # Guard to prevent multiple inclusion
        FetchContent_Declare(
            glfw
            GIT_REPOSITORY https://github.com/glfw/glfw.git
            GIT_TAG 3.3.8
        )
        if(NOT glfw_POPULATED)
            set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
            set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
            set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
            FetchContent_Populate(glfw)
        endif()

        add_subdirectory(${glfw_SOURCE_DIR})
        include_directories(${GLFW_SOURCE_DIR}/include)
    endif()
endmacro()

# Macro to import glm
macro(import_glm)
    if(NOT TARGET glm)  # Guard to prevent multiple inclusion
        FetchContent_Declare(
            glm
            GIT_REPOSITORY https://github.com/g-truc/glm.git
            GIT_TAG 0.9.9.8
        )
        FetchContent_MakeAvailable(glm)

        include_directories(${glm_SOURCE_DIR})
    endif()
endmacro()

# Macro to import glew
macro(import_glew)
    if(NOT TARGET glew_s)  # Guard to prevent multiple inclusion
        include(FetchContent)

        FetchContent_Declare(
            glew
            GIT_REPOSITORY https://github.com/omniavinco/glew-cmake.git
            GIT_TAG master  # Or use a specific commit or tag if preferred
        )

        # Disable shared builds if needed
        set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libs" FORCE)
        FetchContent_MakeAvailable(glew)

        # Optional: include directories if needed manually (though target_link_libraries handles it)
        include_directories(${glew_SOURCE_DIR}/include)
    endif()
endmacro()

# Macro to import entt
macro(import_entt)
    if(NOT TARGET entt)  # Guard to prevent multiple inclusion
        FetchContent_Declare(
            entt
            GIT_REPOSITORY https://github.com/skypjack/entt.git
            GIT_TAG v3.13.2
        )
        FetchContent_MakeAvailable(entt)
        
        include_directories(${entt_SOURCE_DIR})
    endif()
endmacro()

# Macro to import imgui
macro(import_imgui)
    if(NOT TARGET imgui)  # Guard to prevent multiple inclusion
        FetchContent_Declare(
            imgui
            GIT_REPOSITORY https://github.com/ocornut/imgui.git
            GIT_TAG v1.89.9
        )
        FetchContent_Populate(imgui)

        # Add ImGui source files
        file(GLOB IMGUI_SOURCES 
            ${imgui_SOURCE_DIR}/*.cpp
        )

        # Define the ImGui library
        add_library(imgui STATIC
            ${IMGUI_SOURCES}
            ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
            ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
        )

        target_include_directories(imgui PUBLIC
            ${imgui_SOURCE_DIR}
            ${imgui_SOURCE_DIR}/backends
        )

        target_link_libraries(imgui PUBLIC glfw libglew_static)
    endif()
endmacro()

# Macro to import assimp
macro(import_assimp)
    if(NOT TARGET assimp)  # Guard to prevent multiple inclusion
        FetchContent_Declare(
            assimp
            GIT_REPOSITORY https://github.com/assimp/assimp.git
            GIT_TAG v5.2.5
        )
        
        # Configure Assimp build options
        set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "" FORCE)
        set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
        set(ASSIMP_INSTALL OFF CACHE BOOL "" FORCE)
        set(ASSIMP_NO_EXPORT ON CACHE BOOL "" FORCE)
        
        FetchContent_MakeAvailable(assimp)
        
        include_directories(${assimp_SOURCE_DIR}/include)
    endif()
endmacro()

# Macro to import all dependencies
macro(importDependencies)
    message(STATUS "Starting to import dependencies...")

    message(STATUS "Importing GLFW...")
    import_glfw()
    message(STATUS "GLFW imported successfully.")

    message(STATUS "Importing GLM...")
    import_glm()
    message(STATUS "GLM imported successfully.")

    message(STATUS "Importing GLEW...")
    import_glew()
    message(STATUS "GLEW imported successfully.")
    
    message(STATUS "Importing EnTT...")
    import_entt()
    message(STATUS "EnTT imported successfully.")

    message(STATUS "Importing ImGui...")
    import_imgui()
    message(STATUS "ImGui imported successfully.")
    
    message(STATUS "Importing Assimp...")
    import_assimp()
    message(STATUS "Assimp imported successfully.")

    message(STATUS "All dependencies have been imported successfully.")
endmacro()