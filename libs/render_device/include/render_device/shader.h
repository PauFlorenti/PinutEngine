#pragma once

namespace RED
{
static constexpr i32 MAX_UNIFORM_SLOTS    = 4;
static constexpr i32 MAX_DESCRIPTOR_SETS  = 2;
static constexpr i32 NUMBER_SHADER_STAGES = 2; // VERTEX AND FRAGMENT

enum class ShaderType
{
    VERTEX,
    FRAGMENT,
    COMPUTE,
    COUNT
};

struct UniformData
{
    UniformData() = delete;
    UniformData(ShaderType shaderType, std::string name, i32 binding, u32 set);

    //! TODO Check if size is necessary here.
    static UniformData Create(ShaderType shaderType, std::string name, i32 binding, u32 set = 0);

    ShaderType  m_shaderType{ShaderType::COUNT};
    std::string m_name;
    i32         m_binding{-1};
    u32         m_set{0};
};

using UniformNames     = std::vector<std::string>;
using UniformDataSlots = std::vector<UniformData>;
using DescriptorSets   = std::vector<u32>;

struct Shader
{
    Shader(const std::string& name_, ShaderType type, const std::string& entryPoint_ = "main");

    const std::string name;
    const std::string entryPoint = "main";
    ShaderType        shaderType = ShaderType::COUNT;

    UniformDataSlots uniformDataSlots;
    DescriptorSets   descriptorSets;
};
} // namespace RED
