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

enum class UniformType
{
    BUFFER,
    TEXTURE,
    COUNT
};

struct UniformData
{
    UniformData() = delete;
    UniformData(ShaderType  shaderType,
                UniformType uniformType,
                std::string name,
                i32         binding,
                u32         count,
                u32         set);

    //! TODO Check if size is necessary here.
    static UniformData Create(ShaderType  shaderType,
                              UniformType uniformType,
                              std::string name,
                              i32         binding,
                              u32         count = 1,
                              u32         set   = 0);

    ShaderType  m_shaderType{ShaderType::COUNT};
    UniformType m_uniformType{UniformType::COUNT};
    std::string m_name;
    i32         m_binding{-1};
    u32         m_count{1};
    u32         m_set{0};
};

using UniformNames     = std::vector<std::string>;
using UniformDataSlots = std::vector<UniformData>;
using DescriptorSets   = std::vector<u32>;

struct Shader
{
    Shader(const std::string& name_, ShaderType type, const std::string& entryPoint_ = "main");
    Shader(const std::string& name_,
           ShaderType         type,
           UniformDataSlots   uniforms,
           DescriptorSets     descriptorSets_,
           const std::string& entryPoint_ = "main");

    const std::string name;
    const std::string entryPoint = "main";
    ShaderType        shaderType = ShaderType::COUNT;

    UniformDataSlots uniformDataSlots;
    DescriptorSets   descriptorSets;
};
} // namespace RED
