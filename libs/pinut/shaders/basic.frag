#version 450

layout(location = 0) in vec3 inNormal;
layout(location = 1) in vec3 inPosition;
layout(location = 2) in vec3 inCameraPosition;

layout(location = 0) out vec4 outColor;

vec3 light_position = vec3(2, 5, 1);
vec3 light_color = vec3(0.8, 0.5, 0.1);

float specular_strengh = 0.5f;

void main()
{
    vec3 N = normalize(inNormal);
    vec3 L = normalize(light_position - inPosition);
    vec3 V = normalize(inCameraPosition - inPosition);
    vec3 R = reflect(-L, N);

    float dotNL = max(dot(N, L), 0.0);
    float dotVR = max(dot(V, R), 0.0);
    vec3 diffuse = light_color * dotNL;
    vec3 specular = specular_strengh * pow(dotVR, 2) * light_color;

    vec3 light = (diffuse + specular) * vec3(1.0, 0.0, 0.0);

    outColor = vec4(light, 1.0);
}
