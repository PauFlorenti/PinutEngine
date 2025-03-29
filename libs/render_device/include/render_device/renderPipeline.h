#pragma once

#include "render_device/shader.h"
#include "render_device/textureFormat.h"

namespace RED
{
struct RenderPipeline
{
    const char* name;

    Shader                     vertexShader;
    Shader                     fragmentShader;
    std::string                inputVertexDeclaration;
    std::vector<TextureFormat> attachmentFormats;
    TextureFormat              depthFormat;

    bool operator==(const RenderPipeline&) const noexcept = default;
};
} // namespace RED
