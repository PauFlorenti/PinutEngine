#version 450

layout (location = 0) in vec3 inPosition;
layout (location = 0) out vec3 outColor;

layout(set = 0, binding = 0) uniform perFrame
{
    mat4 view;
    mat4 projection;
    vec3 cameraPosition;
} perFrameData;

void main()
{
    mat4 view = perFrameData.view;
    mat4 projection = perFrameData.projection;

    vec4 world_position = mat4(1) * vec4(inPosition, 1.0);

    outColor = vec3(0, 1, 0);
    gl_Position = projection * view * world_position;
}
