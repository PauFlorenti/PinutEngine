#version 450

layout (location = 0) in vec2 inUv;

layout (location = 0) out vec4 fragColor;

layout (set = 0, binding = 0) uniform sampler2D colorTexture;

void main()
{
    fragColor = texture(colorTexture, inUv);
}