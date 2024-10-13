#pragma once

#include "render_device/shader.h"

class ShaderFlatVS
{
  public:
    static constexpr const char* const     name       = "shaders/flat.vert";
    static constexpr const char* const     entryPoint = "main";
    static constexpr const RED::ShaderType shaderType{RED::ShaderType::VERTEX};
};
