#pragma once

#include "render_device/buffer.h"
#include "render_device/shader.h"
#include "render_device/texture.h"

namespace RED
{
struct UniformDescriptor
{
    GPUBufferView  bufferView;
    GPUTextureView textureView;
    ShaderType     shaderType{ShaderType::COUNT};
    u32            binding{0};
    u32            set{0};
    const char*    name = "";
};

struct DrawCall
{
    DrawCall();

    void SetUniformBuffer(GPUBufferView bufferView,
                          ShaderType    shaderType,
                          u32           binding,
                          u32           set = 0);
    void SetUniformBuffer(const std::string& name, ShaderType shaderType, GPUBufferView bufferView);
    void SetUniformTexture(GPUTextureView textureView,
                           ShaderType     shaderType,
                           u32            binding,
                           u32            set = 0);

    GPUBufferView vertexBuffer;
    GPUBufferView indexBuffer;

    // TODO Hardcoded as assuming 2 uniform buffers per shader.
    // 0 and 1 for Vertex shader 2 and 3 for Pixel shader.
    // std::array<UniformDescriptor,
    //            16 /*NUMBER_SHADER_STAGES * MAX_DESCRIPTOR_SETS * MAX_UNIFORM_SLOTS*/>
    //   uniforms;
    std::vector<UniformDescriptor> uniforms;
};
} // namespace RED
