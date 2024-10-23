#include "pch.hpp"

#include "render_device/drawCall.h"

namespace RED
{

DrawCall::DrawCall() { uniforms.reserve(16); }

void DrawCall::SetUniformBuffer(GPUBufferView bufferView,
                                ShaderType    shaderType,
                                u32           binding,
                                u32           set)
{
    assert(set < 2 /*MAX_DESCRIPTOR_SETS*/);
    assert(binding < 4 /*MAX_UNIFORM_SLOTS*/);

    UniformDescriptor uniform{bufferView, shaderType, binding, set, ""};
    uniforms.emplace_back(std::move(uniform));
}

void DrawCall::SetUniformBuffer(const std::string& name,
                                ShaderType         shaderType,
                                GPUBufferView      bufferView)
{
    UniformDescriptor uniform{bufferView, shaderType, 0, 0, name.c_str()};
    uniforms.emplace_back(std::move(uniform));
}
} // namespace RED
