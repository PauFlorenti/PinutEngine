#include "pch.hpp"

#include "render_device/shader.h"

namespace RED
{
UniformData UniformData::Create(ShaderType shaderType, std::string name, i32 binding, u32 set)
{
    return {shaderType, name, binding, set};
}

UniformData::UniformData(ShaderType shaderType, std::string name, i32 binding, u32 set)
: m_shaderType(shaderType),
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
} // namespace RED
