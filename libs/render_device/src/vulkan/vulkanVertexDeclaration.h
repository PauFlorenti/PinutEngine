#pragma once

namespace RED
{
namespace vulkan
{
struct VertexDeclaration
{
    u32                                                  stride = 0;
    const std::vector<VkVertexInputAttributeDescription> layout;
    const std::string                                    name;

    VertexDeclaration(const char*                                           name_,
                      const std::vector<VkVertexInputAttributeDescription>& layout_,
                      u32                                                   stride_);
};

const VertexDeclaration* getVertexDeclarationByName(const std::string& name);

} // namespace vulkan
} // namespace RED
