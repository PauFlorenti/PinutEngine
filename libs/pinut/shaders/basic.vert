#version 460

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec4 inColor;
layout (location = 2) in vec2 inUv;
layout (location = 3) in vec3 inNormal;

layout (location = 0) out vec3 outNormal;
layout (location = 1) out vec3 outPosition;
layout (location = 2) out vec3 outCameraPosition;
layout (location = 3) out vec4 outColor;
layout (location = 4) out vec2 outUv;

layout(set = 0, binding = 0) uniform perFrame
{
    mat4 view;
    mat4 projection;
    vec3 cameraPosition;
} perFrameData;

layout (set = 1, binding = 0) uniform perInstance
{
    mat4 model;
} instanceData;

void main() 
{
    mat4 view       = perFrameData.view;
    mat4 projection = perFrameData.projection;
    mat4 model      = instanceData.model;

    vec4 world_position = model * vec4(inPosition, 1.0);
    vec3 N = mat3(transpose(inverse(model))) * inNormal;

    gl_Position = projection * view * world_position;

    outNormal = N;
    outPosition = world_position.xyz;
    outCameraPosition = perFrameData.cameraPosition;
    outColor = inColor;
    outUv = inUv;
}