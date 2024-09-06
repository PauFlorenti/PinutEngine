#version 450

layout (location = 0) in vec3 position;

layout (location = 0) out vec3 out_color;

layout (binding = 0) uniform UniformBuffer {
    mat4 view;
    mat4 proj;
} ubo;

layout (push_constant) uniform push_constant
{
    mat4 model;
    vec3 color;
} u_push_constant;

void main()
{
    mat4 view_projection = ubo.proj * ubo.view;
    vec4 world_position = u_push_constant.model * vec4(position, 1.0f);

    out_color = u_push_constant.color;
    gl_Position = view_projection * world_position;
}
