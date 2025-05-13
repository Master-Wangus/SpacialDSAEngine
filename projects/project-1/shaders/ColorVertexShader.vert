#version 410 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aColor;

uniform mat4 vertexTransform;

out vec3 vColor;

void main()
{
    gl_Position = vertexTransform * vec4(aPosition, 1.0);
    vColor = aColor;
}
