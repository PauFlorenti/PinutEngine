#version 460

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec4 inColor;
layout (location = 2) in vec2 inUv;
layout (location = 3) in vec3 inNormal;

layout(location = 0) out vec2 outUv;
layout(location = 1) out vec3 outColor;

layout(set = 0, binding = 0) uniform SkyboxData
{
    mat4 view;
    mat4 projection;
    mat4 model;
    vec4 color;
} skyboxData;

void main() 
{
    vec4 position = skyboxData.projection * skyboxData.view * skyboxData.model * vec4(inPosition, 1.0);
    gl_Position = position.xyww;

    outColor    = skyboxData.color.xyz;
    outUv       = inUv * -1;
}