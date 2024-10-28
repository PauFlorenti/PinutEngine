#version 450

layout (location = 0) in vec3 in_color;
layout (location = 1) in vec3 in_normal;

layout (location = 0) out vec4 out_color;

void main()
{
    float NdotL = clamp(dot(in_normal, vec3(0, 1, 0)), 0.0f, 1.0f);
    out_color = NdotL * vec4(in_color, 1);
}
