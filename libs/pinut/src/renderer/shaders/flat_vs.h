#pragma once

#include "render_device/src/shader.h"

class ShaderFlatVS
{
    static constexpr const std::string name       = "shaders/flat.vs";
    static constexpr const std::string entryPoint = "main";
    static constexpr const ShaderType  shaderType{ShaderType::VERTEX};
};
