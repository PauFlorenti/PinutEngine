#version 460

#extension GL_GOOGLE_include_directive : enable

#include "helpers.glsl"

layout(location = 0) in vec3 inNormal;
layout(location = 1) in vec3 inPosition;
layout(location = 2) in vec3 inCameraPosition;
layout(location = 3) in vec4 inColor;
layout(location = 4) in vec2 inUv;

layout(location = 0) out vec4 outColor;

struct Light
{
    vec3    color;
    float   intensity;
    vec3    position;
    float   radius;
    vec3    direction; // Only used in spotlight.
    float   innerCone; // Radians.
    float   outerCone; // Radians. If 2 * PI (180), then pointlight.
    float   cosineExponent; // Only used in spotlight.
    float   dummy1;
    float   dummy2;
};

struct DirectionalLight
{
    vec3    direction;
    float   intensity;
    vec3    color;
    float   dummy;
};

layout(set = 0, binding = 1) uniform perFrame
{
    Light lights[4];
    DirectionalLight directionalLight;
} lightData;

layout(set = 1, binding = 1) uniform perInstance
{
    uint diffuse;
    uint specularExponent;
    uint dummy1;
    uint dummy2;
} perInstanceData;

layout(set = 1, binding = 2) uniform sampler2D[5] materialTextures;

float ambientLightIntensity = 0.01;

vec4 UnpackColor(uint packedColor)
{
    vec4 unpackedColor;
    unpackedColor.r = float((packedColor & uint(0x000000FF)) >> 0) / 255.0;
    unpackedColor.g = float((packedColor & uint(0x0000FF00)) >> 8) / 255.0;
    unpackedColor.b = float((packedColor & uint(0x00FF0000)) >> 16) / 255.0;
    unpackedColor.a = float((packedColor & uint(0xFF000000)) >> 24) / 255.0;
    return unpackedColor;
}

vec3 ComputeDirectionalLight(DirectionalLight l, vec3 N, vec3 materialDiffuseColor)
{
    vec3 color = vec3(0.0f);

    if (l.intensity <= 1e-6 || length(l.direction) < 1e-6)
    {
        return color;
    }

    vec3 L = normalize(l.direction);
    float dotNL = max(dot(N, L), 0.0f);

    return l.intensity * dotNL * materialDiffuseColor;
}

void main()
{
    const vec3 N        = normalize(inNormal);
    const vec3 V        = normalize(inCameraPosition - inPosition);
    const float NdotV   = max(dot(N, V), 0.0f);

    const vec3 diffuseTexture             = texture(materialTextures[0], inUv).xyz;
    const vec3 normalTexture              = texture(materialTextures[1], inUv).xyz;
    const vec3 roughnessMetallicTexture   = texture(materialTextures[2], inUv).xyz;
    const vec3 emissiveTexture            = texture(materialTextures[3], inUv).xyz;

    const float roughness   = roughnessMetallicTexture.y;
    const float metallic    = roughnessMetallicTexture.z;
    const vec3 F0           = mix(vec3(0.04), diffuseTexture, metallic);

    vec3 materialDiffuseColor   = diffuseTexture * inColor.xyz * UnpackColor(perInstanceData.diffuse).xyz;

    vec3 ambient = vec3(0.0f);
    vec3 diffuse = vec3(0.0f);
    vec3 specular = vec3(0.0f);

    vec3 Lo = vec3(0.0);
    Lo += ComputeDirectionalLight(lightData.directionalLight, N, materialDiffuseColor);

    for (int i = 0; i < 4; ++i)
    {
        Light light = lightData.lights[i];

        const vec3 lightPosition    = light.position;
        const vec3 lightColor       = light.color;
        const vec3 lightDirection   = light.direction;
        const float lightIntensity  = light.intensity;
        const float lightRadius     = light.radius;

        // If near zero, skip it.
        if (lightIntensity < 1e-6f)
            continue;

        vec3 L                  = lightPosition - inPosition;
        float lightDistance     = length(L);
        
        if (lightDistance > lightRadius)
            continue;

        float attenuation = lightRadius - lightDistance;
        attenuation /= lightRadius;
        attenuation = max(attenuation, 0.0f);
        attenuation = attenuation * attenuation;

        L                   = normalize(L);
        const vec3 R        = reflect(-L, N);
        const vec3 H        = normalize(V + L);
        const float NdotL   = max(dot(N, L), 0.0);
        const float NdotH   = max(dot(N, H), 0.0);
        const float VdotR   = max(dot(V, R), 0.0);

        // Compute spot factor.
        float spotFactor = 1.0f;
        if (light.outerCone > 0)
        {
            vec3 D          = normalize(light.direction);
            float theta     = dot(-L, D);
            spotFactor      = 0.0f;

            if (theta > cos(light.outerCone))
            {
                spotFactor = smoothstep(cos(light.outerCone), cos(light.innerCone), theta);
                spotFactor = pow(spotFactor, light.cosineExponent);
            }
        }

        attenuation *= spotFactor;

        const vec3 radiance = lightIntensity * lightColor * attenuation; // * shadowFactor;
        const vec3 F        = FresnelSchlick(NdotH, F0);
        const float D       = DistributionGGX(N, H, roughness);
        const float G       = GeometrySmith(N, V, L, roughness);

        const vec3 numerator    = D * G * F;
        const float denominator = max(4.0 * NdotV * NdotL, 0.0000001);
        specular += numerator / denominator;

        const vec3 kS = F;
        const vec3 kD = (vec3(1.0) - kS) * (1.0 - metallic);

        Lo += (kD * diffuseTexture / PI + specular) * radiance * NdotL;
    }

    outColor = vec4(Lo + emissiveTexture, 1.0);
}
