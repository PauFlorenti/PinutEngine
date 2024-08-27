#version 460

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inUv;

layout(location = 0) out vec2 outUv;

layout(set = 0, binding = 0) uniform perFrame
{
    mat4 view;
    mat4 projection;
    vec3 cameraPosition;
} perFrameData;

layout(set = 0, binding = 1) readonly buffer transformsBuffer
{
    mat4 models[];
} transforms;

void main() 
{
    mat4 view = perFrameData.view;
    mat4 projection = perFrameData.projection;
    mat4 model = transforms.models[gl_BaseInstance];

    vec4 world_position = model * vec4(inPosition, 1.0);
    gl_Position = projection * view * world_position;

    outUv = inUv;
}