#version 460

layout(location = 0) in vec2 inUv;

layout(location = 0) out vec4 outColor;

layout(set = 1, binding = 0) readonly buffer perInstance
{
    uint diffuse;
    uint specularExponent;
    uint dummy1;
    uint dummy2;
} perInstanceData;

layout(set = 1, binding = 1) uniform sampler2D diffuseTexture;

vec4 UnpackColor(uint packedColor)
{
    vec4 unpackedColor;
    unpackedColor.r = float((packedColor & uint(0x000000FF)) >> 0) / 255.0;
    unpackedColor.g = float((packedColor & uint(0x0000FF00)) >> 8) / 255.0;
    unpackedColor.b = float((packedColor & uint(0x00FF0000)) >> 16) / 255.0;
    unpackedColor.a = float((packedColor & uint(0xFF000000)) >> 24) / 255.0;
    return unpackedColor;
}

void main()
{
    vec4 diffuseTextureValue    = texture(diffuseTexture, inUv);
    vec4 materialDiffuseColor   = diffuseTextureValue * UnpackColor(perInstanceData.diffuse);

    outColor = materialDiffuseColor;
}
