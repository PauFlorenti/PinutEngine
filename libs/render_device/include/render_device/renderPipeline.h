#pragma once

#include "render_device/shader.h"

namespace RED
{
struct RenderPipeline
{
    const char* name;

    const Shader& vertexShader;
    const Shader& fragmentShader;

    bool operator==(const RenderPipeline&) const noexcept = default;
};
} // namespace RED
