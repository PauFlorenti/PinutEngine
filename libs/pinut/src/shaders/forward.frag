#version 450

layout(location = 0) in vec4 inColor;
layout(location = 1) in vec3 inPosition;
layout(location = 2) in vec2 inUv;

layout(location = 0) out vec4 outColor;

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

layout(set = 1, binding = 1) uniform sampler2D textures[3];

vec3 lightPosition = vec3(0.0, 1.0, -1.0);
vec3 lightColor = vec3(1.0, 1.0, 1.0);
float lightRadius = 3.0f;

void main() 
{
    // textures
    vec4 normal_texture                 = texture(textures[1], inUv);
    vec4 albedo_texture                 = inColor * texture(textures[0], inUv);
    vec4 metallic_roughness_texture    = texture(textures[2], inUv);

    float metallic = metallic_roughness_texture.z;
    float roughness = metallic_roughness_texture.y;

    vec3 N = normalize(mat3(transpose(perObjectData.inverse_model)) * normal_texture.xyz);
    vec3 L = normalize(lightPosition - inPosition);

    float NdotL = max(dot(N, L), 0.0f);
    vec3 diffuse = NdotL * lightColor;

    outColor = vec4(diffuse, 1) * albedo_texture;
}
