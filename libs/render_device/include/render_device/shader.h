#pragma once

namespace RED
{
enum class ShaderType
{
    VERTEX,
    FRAGMENT,
    COMPUTE,
    COUNT
};

struct Shader
{
    const std::string name;
    const std::string entryPoint = "main";

    ShaderType shaderType = ShaderType::COUNT;
};
} // namespace RED
