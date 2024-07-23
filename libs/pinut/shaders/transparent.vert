#version 460

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec4 inColor;
layout(location = 3) in vec2 inUv;

layout(location = 0) out vec3 outNormal;
layout(location = 1) out vec3 outPosition;
layout(location = 2) out vec3 outCameraPosition;
layout(location = 3) out vec4 outColor;
layout(location = 4) out vec2 outUv;

layout(set = 0, binding = 0) uniform perFrame
{
    mat4 view;
    mat4 projection;
    vec3 cameraPosition;
} perFrameData;

layout(set = 0, binding = 1) readonly buffer transformsBuffer
{
    mat4 models[];
} transforms;

layout(set = 1, binding = 0) uniform perInstance
{
    vec3 padding;
    uint color;
} perInstanceData;

vec4 unpackColor(uint packedColor)
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
    mat4 view = perFrameData.view;
    mat4 projection = perFrameData.projection;
    mat4 model = transforms.models[gl_BaseInstance];
    vec4 unpackedColor = unpackColor(perInstanceData.color);

    vec4 world_position = model * vec4(inPosition, 1.0);
    vec3 N = mat3(transpose(inverse(model))) * inNormal;

    gl_Position = projection * view * world_position;

    vec4 vertexColor = unpackedColor * inColor;

    outNormal = N;
    outPosition = world_position.xyz;
    outCameraPosition = perFrameData.cameraPosition;
    outColor = vertexColor;
    // outColor = vec4(0.0f, 0.0f, 1.0f, 0.5f);
    outUv = inUv;
}