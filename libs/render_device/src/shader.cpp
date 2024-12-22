#include "pch.hpp"

#include "render_device/shader.h"

namespace RED
{
UniformData UniformData::Create(ShaderType  shaderType,
                                UniformType uniformType,
                                std::string name,
                                i32         binding,
                                u32         set)
{
    return {shaderType, uniformType, name, binding, set};
}

UniformData::UniformData(ShaderType  shaderType,
                         UniformType uniformType,
                         std::string name,
                         i32         binding,
                         u32         set)
: m_shaderType(shaderType),
  m_uniformType(uniformType),
  m_name(name),
  m_binding(binding),
  m_set(set)
{
}

Shader::Shader(const std::string& name_, ShaderType type, const std::string& entryPoint_)
: name(name_),
  shaderType(type),
  entryPoint(entryPoint_)
{
    descriptorSets.reserve(MAX_DESCRIPTOR_SETS * NUMBER_SHADER_STAGES); // 4 * 2
}

Shader::Shader(const std::string& name_,
               ShaderType         type,
               UniformDataSlots   uniforms,
               DescriptorSets     descriptorSets_,
               const std::string& entryPoint_)
: name(name_),
  shaderType(type),
  uniformDataSlots(std::move(uniforms)),
  descriptorSets(std::move(descriptorSets_)),
  entryPoint(entryPoint_)
{
    assert(descriptorSets.size() < MAX_DESCRIPTOR_SETS * NUMBER_SHADER_STAGES);
}
} // namespace RED
