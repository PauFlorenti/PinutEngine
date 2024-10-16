#include "pch.hpp"

#include "src/vulkan/vulkanVertexDeclaration.h"

namespace RED
{
namespace vulkan
{
static std::vector<VkVertexInputAttributeDescription> layoutPosColorUVsNorm = {
  {0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0},
  {1, 0, VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(f32) * 3},
  {2, 0, VK_FORMAT_R32G32_SFLOAT, sizeof(f32) * 7},
  {3, 0, VK_FORMAT_R32G32B32_SFLOAT, sizeof(f32) * 9}};

VertexDeclaration vtx_decl_pos_color_uvs_norm("PosColorUvN", layoutPosColorUVsNorm, 48);

static std::vector<VkVertexInputAttributeDescription> layoutPos = {
  {0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0}};

VertexDeclaration vtx_decl_pos("Pos", layoutPos, 12);

static std::vector<VkVertexInputAttributeDescription> layoutPosUV = {
  {0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0},
  {1, 0, VK_FORMAT_R32G32_SFLOAT, sizeof(f32) * 3}};

VertexDeclaration vtx_decl_pos_uv("PosUv", layoutPosUV, 20);

VertexDeclaration::VertexDeclaration(const char*                                           name_,
                                     const std::vector<VkVertexInputAttributeDescription>& layout_,
                                     u32                                                   stride_)
: name(name_),
  layout(layout_),
  stride(stride_) {};

const VertexDeclaration* getVertexDeclarationByName(const std::string& name)
{
    if (name == vtx_decl_pos.name)
        return &vtx_decl_pos;
    if (name == vtx_decl_pos_uv.name)
        return &vtx_decl_pos_uv;
    if (name == vtx_decl_pos_color_uvs_norm.name)
        return &vtx_decl_pos_color_uvs_norm;
    return nullptr;
}
} // namespace vulkan
} // namespace RED
