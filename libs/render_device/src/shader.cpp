#include "pch.hpp"

#include "render_device/shader.h"

namespace RED
{
Shader::Shader(const std::string& name_, ShaderType type, const std::string& entryPoint_)
: name(name_),
  shaderType(type),
  entryPoint(entryPoint_)
{
}
} // namespace RED
