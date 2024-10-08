#pragma once

struct RasterState
{
    VkPolygonMode   polygonMode{VK_POLYGON_MODE_FILL};
    VkCullModeFlags cullingMode{VK_CULL_MODE_NONE};
    VkFrontFace     frontFace{VK_FRONT_FACE_COUNTER_CLOCKWISE};

    bool operator==(const RasterState&) const noexcept = default;
};

struct BlendState
{
    VkBlendFactor            srcAlpha{VK_BLEND_FACTOR_MAX_ENUM};
    VkBlendFactor            srcColor{VK_BLEND_FACTOR_MAX_ENUM};
    VkBlendFactor            dstAlpha{VK_BLEND_FACTOR_MAX_ENUM};
    VkBlendFactor            dstColor{VK_BLEND_FACTOR_MAX_ENUM};
    VkBlendOp                colorBlendOp{VK_BLEND_OP_MAX_ENUM};
    VkBlendOp                alphaBlendOp{VK_BLEND_OP_MAX_ENUM};
    VkColorComponentFlagBits writeMask{VK_COLOR_COMPONENT_FLAG_BITS_MAX_ENUM};

    bool operator==(const BlendState&) const noexcept = default;
};

struct GraphicsState
{
    RasterState         raster{};
    BlendState          blend{};
    VkPrimitiveTopology topology{VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST};

    bool operator==(const GraphicsState&) const noexcept = default;
};

namespace std
{
template <>
struct hash<RasterState>
{
    inline size_t operator()(const RasterState& state) const noexcept
    {
        size_t h1 = std::hash<VkPolygonMode>{}(state.polygonMode);
        size_t h2 = std::hash<VkCullModeFlags>{}(state.cullingMode);
        size_t h3 = std::hash<VkFrontFace>{}(state.frontFace);
    }
};

template <>
struct hash<BlendState>
{
    inline size_t operator()(const BlendState& state) const noexcept
    {
        size_t h1 = std::hash<VkBlendFactor>{}(state.srcAlpha);
        size_t h2 = std::hash<VkBlendFactor>{}(state.srcColor);
        size_t h3 = std::hash<VkBlendFactor>{}(state.dstAlpha);
        size_t h4 = std::hash<VkBlendFactor>{}(state.dstColor);
        size_t h5 = std::hash<VkBlendOp>{}(state.colorBlendOp);
        size_t h6 = std::hash<VkBlendOp>{}(state.alphaBlendOp);
        size_t h7 = std::hash<VkColorComponentFlagBits>{}(state.writeMask);

        return h1 ^ (h2 << 1) ^ (h3 << 2) ^ (h4 << 4) ^ (h5 << 8) ^ (h6 << 16) ^ (h7 << 32);
    }
};

template <>
struct hash<GraphicsState>
{
    inline size_t operator()(const GraphicsState& state) const noexcept
    {
        size_t h1 = std::hash<RasterState>{}(state.raster);
        size_t h2 = std::hash<BlendState>{}(state.blend);
        size_t h3 = std::hash<VkPrimitiveTopology>{}(state.topology);

        return h1 ^ (h2 << 1) ^ (h3 << 2);
    }
};
} // namespace std
