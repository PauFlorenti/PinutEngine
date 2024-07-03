#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUv;
layout(location = 3) in vec4 inColor;

layout(location = 0) out vec3 outNormal;
layout(location = 1) out vec3 outPosition;

layout(set = 0, binding = 0) uniform perFrame
{
    mat4 view;
    mat4 projection;
} perFrameData;

layout(set = 1, binding = 0) uniform perInstance
{
    mat4 model;
} perInstanceData;

void main() 
{
    mat4 view = perFrameData.view;
    mat4 projection = perFrameData.projection;
    mat4 model = perInstanceData.model;

    vec4 world_position = model * vec4(inPosition, 1.0);
    vec3 N = mat3(transpose(inverse(model))) * inNormal;

    outNormal = N;
    outPosition = world_position.xyz;

    gl_Position = projection * view * world_position;
}