#include "pch.hpp"

#include "render_device/drawCall.h"

namespace RED
{

DrawCall::DrawCall() { uniforms.reserve(16); }

void DrawCall::SetUniformBuffer(std::vector<GPUBufferView> bufferViews,
                                ShaderType                 shaderType,
                                u32                        binding,
                                u32                        set)
{
    assert(set < 2 /*MAX_DESCRIPTOR_SETS*/);
    assert(binding < 4 /*MAX_UNIFORM_SLOTS*/);

    UniformDescriptor uniform{std::move(bufferViews), {}, shaderType, binding, set, ""};
    uniforms.emplace_back(std::move(uniform));
}

void RED::DrawCall::SetUniformTexture(std::vector<GPUTextureView> textureViews,
                                      ShaderType                  shaderType,
                                      u32                         binding,
                                      u32                         set)
{
    assert(set < 2 /*MAX_DESCRIPTOR_SETS*/);
    assert(binding < 4 /*MAX_UNIFORM_SLOTS*/);

    UniformDescriptor uniform{{}, std::move(textureViews), shaderType, binding, set, ""};
    uniforms.emplace_back(std::move(uniform));
}
} // namespace RED
