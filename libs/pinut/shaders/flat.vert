#version 450

layout (location = 0) in vec3 inPosition;
layout (location = 0) out vec3 outColor;

void main()
{
    outColor = vec3(0, 1, 0);
    gl_Position = vec4(inPosition, 1.0);
}
