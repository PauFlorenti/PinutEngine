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
    Shader(const std::string& name_, ShaderType type, const std::string& entryPoint_ = "main");

    const std::string name;
    const std::string entryPoint = "main";

    ShaderType shaderType = ShaderType::COUNT;
};
} // namespace RED
