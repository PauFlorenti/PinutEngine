#version 460

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
};

layout(set = 0, binding = 2) uniform perFrame
{
    uint count;
    Light lights[10];
} lightData;

layout(set = 1, binding = 1) uniform sampler2D diffuseTexture;

float specular_strengh = 0.5f;

void main()
{
    vec3 N = normalize(inNormal);
    vec3 V = normalize(inCameraPosition - inPosition);

    vec3 diffuseTextureValue = texture(diffuseTexture, inUv).xyz;
    vec3 materialColor = diffuseTextureValue * inColor.xyz;

    vec3 output_light = vec3(0.0f);
    for (int i = 0; i < lightData.count; ++i)
    {
        Light light = lightData.lights[i];

        vec3 light_position     = light.position;
        vec3 light_color        = light.color;
        float light_intensity   = light.intensity;
        float light_radius      = light.radius;

        // If near zero, skip it.
        if (light_intensity < 0.001f)
            continue;

        vec3 L                  = light_position - inPosition;
        float light_distance    = length(L);
        
        if (light_distance > light_radius)
            continue;

        float attenuation_factor = light_radius - light_distance;
        attenuation_factor /= light_radius;
        attenuation_factor = max(attenuation_factor, 0.0f);
        attenuation_factor = attenuation_factor * attenuation_factor;

        L                       = normalize(L);
        vec3 R                  = reflect(-L, N);

        float dotNL = max(dot(N, L), 0.0);
        float dotVR = max(dot(V, R), 0.0);

        vec3 diffuse = light_color * dotNL;
        vec3 specular = specular_strengh * pow(dotVR, 2) * light_color;

        output_light += (diffuse + specular) * attenuation_factor * materialColor;
    }

    outColor = vec4(output_light, 1.0);
}
