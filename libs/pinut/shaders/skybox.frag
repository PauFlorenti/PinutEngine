#version 460

layout(location = 0) in vec2 inUv;

layout(location = 0) out vec4 outColor;

layout(set = 1, binding = 0) uniform sampler2D skyboxTexture;

void main()
{
    outColor = vec4(texture(skyboxTexture, inUv).xyz, 0.0f);
}
