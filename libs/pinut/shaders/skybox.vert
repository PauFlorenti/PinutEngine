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

layout( push_constant ) uniform SkyboxModel {
    mat4 model;
};

void main() 
{
    gl_Position = perFrameData.projection * perFrameData.view * model * vec4(inPosition, 1.0);
    outUv = inUv;
}