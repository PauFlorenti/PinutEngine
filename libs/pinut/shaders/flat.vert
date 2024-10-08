#version 450

layout (location = 0) in vec3 position;

layout (location = 0) out vec3 outColor;

vec3 vertices[3] = {
    {-0.5f, -0.5f, 0.0f},
    { 0.5f, -0.5f, 0.0f},
    { 0.0f,  0.5f, 0.0f}
};

void main()
{
    outColor = vertices[gl_VertexIndex];
    gl_Position = vec4(vertices[gl_VertexIndex], 1.0);
}
