#pragma once

#include "render_device/src/shader.h"

class ShaderFlatFS
{
    static constexpr const std::string name       = "shaders/flat.fs";
    static constexpr const std::string entryPoint = "main";
    static constexpr const ShaderType  shaderType{ShaderType::FRAGMENT};
};
