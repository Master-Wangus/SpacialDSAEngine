#version 330 core

layout (triangles) in;
layout (line_strip, max_vertices = 6) out;

// Input from vertex shader
in vec3 FragPos[];
in vec3 Normal[];
in vec3 Color[];
in vec2 TexCoord[];

// Output to fragment shader
out vec3 FragPosOut;
out vec3 NormalOut;
out vec3 ColorOut;
out vec2 TexCoordOut;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    // For each triangle, emit 3 lines (6 vertices total)
    
    // Line 1: vertex 0 -> vertex 1
    gl_Position = projection * view * model * gl_in[0].gl_Position;
    FragPosOut = FragPos[0];
    NormalOut = Normal[0];
    ColorOut = Color[0];
    TexCoordOut = TexCoord[0];
    EmitVertex();
    
    gl_Position = projection * view * model * gl_in[1].gl_Position;
    FragPosOut = FragPos[1];
    NormalOut = Normal[1];
    ColorOut = Color[1];
    TexCoordOut = TexCoord[1];
    EmitVertex();
    
    EndPrimitive();
    
    // Line 2: vertex 1 -> vertex 2
    gl_Position = projection * view * model * gl_in[1].gl_Position;
    FragPosOut = FragPos[1];
    NormalOut = Normal[1];
    ColorOut = Color[1];
    TexCoordOut = TexCoord[1];
    EmitVertex();
    
    gl_Position = projection * view * model * gl_in[2].gl_Position;
    FragPosOut = FragPos[2];
    NormalOut = Normal[2];
    ColorOut = Color[2];
    TexCoordOut = TexCoord[2];
    EmitVertex();
    
    EndPrimitive();
    
    // Line 3: vertex 2 -> vertex 0
    gl_Position = projection * view * model * gl_in[2].gl_Position;
    FragPosOut = FragPos[2];
    NormalOut = Normal[2];
    ColorOut = Color[2];
    TexCoordOut = TexCoord[2];
    EmitVertex();
    
    gl_Position = projection * view * model * gl_in[0].gl_Position;
    FragPosOut = FragPos[0];
    NormalOut = Normal[0];
    ColorOut = Color[0];
    TexCoordOut = TexCoord[0];
    EmitVertex();
    
    EndPrimitive();
} 