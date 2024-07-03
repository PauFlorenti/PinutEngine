#version 450

layout(location = 0) in vec3 inNormal;
layout(location = 1) in vec3 inPosition;

layout(location = 0) out vec4 outColor;

vec3 light_position = vec3(2, 5, 1);

void main()
{
    vec3 N = normalize(inNormal);
    vec3 L = normalize(light_position - inPosition);
    float dotNL = max(dot(N, L), 0.0);

    outColor = vec4(1.0, 0.0, 0.0, 1.0) * dotNL;
}