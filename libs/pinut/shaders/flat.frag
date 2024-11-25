#version 450

layout (location = 0) in vec3 in_color;
layout (location = 1) in vec3 in_normal;
layout (location = 2) in vec2 in_uv;

layout (location = 0) out vec4 out_color;

layout (set = 1, binding = 1) uniform sampler2D difuse_texture;

void main()
{
    float NdotL = clamp(dot(in_normal, vec3(0, 1, 0)), 0.0f, 1.0f);
    vec4 color  = vec4(in_color, 1) * texture(difuse_texture, in_uv);

    out_color = NdotL * color;
}
