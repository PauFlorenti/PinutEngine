#pragma once

#include "render_device/shader.h"

class ShaderFlatFS
{
  public:
    static constexpr const char* const     name       = "shaders/flat.frag";
    static constexpr const char* const     entryPoint = "main";
    static constexpr const RED::ShaderType shaderType{RED::ShaderType::FRAGMENT};
};
