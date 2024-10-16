#pragma once

#include "render_device/shader.h"

namespace RED
{
struct RenderPipeline
{
    const char* name;

    Shader      vertexShader;
    Shader      fragmentShader;
    std::string inputVertexDeclaration;

    bool operator==(const RenderPipeline&) const noexcept = default;
};
} // namespace RED
