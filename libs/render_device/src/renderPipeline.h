#pragma once

struct Shader;

struct RenderPipeline
{
    const Shader& vertexShader;
    const Shader& fragmentShader;

    bool operator==(const RenderPipeline&) const noexcept;
};
