/**
 * @file my-project-3.vert
 * @brief Vertex shader for 3D object rendering with lighting support.
 *
 * This shader transforms vertices from model space to clip space and passes
 * position, normal, color, and texture coordinates to the fragment shader.
 */

#version 460 core

// Input vertex attributes
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in vec2 aTexCoord;

// Output to fragment shader
out vec3 FragPos;
out vec3 Normal;
out vec3 Color;
out vec2 TexCoord;

// Transformation matrices
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    FragPos = vec3(model * vec4(aPos, 1.0));
    
    // Calculate normal in world space (excluding translation)
    Normal = mat3(transpose(inverse(model))) * aNormal;
    
    Color = aColor;
    TexCoord = aTexCoord;
    
    gl_Position = projection * view * model * vec4(aPos, 1.0);
} 