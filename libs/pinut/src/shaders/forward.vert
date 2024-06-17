#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec4 color;
layout(location = 3) in vec2 uv;

layout(set = 0, binding = 0) uniform perFrame
{
    mat4 view;
    mat4 projection;
} perFrameData;

layout(set = 1, binding = 0) uniform perObject
{
    mat4 model;
    mat4 inverse_model;
} perObjectData;

layout(location = 0) out vec4 out_color;
layout(location = 1) out vec3 out_position;
layout(location = 2) out vec2 out_uv;

void main() {
    out_color = color;
    out_uv = uv;

    vec4 world_position = perObjectData.model * vec4(position, 1.0f);
    out_position = world_position.xyz;

    gl_Position = perFrameData.projection * perFrameData.view * world_position;
}
