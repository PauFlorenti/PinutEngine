#pragma once

bool operator==(const VkVertexInputAttributeDescription& lhs,
                const VkVertexInputAttributeDescription& rhs) noexcept;

namespace RED
{
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

struct DepthState
{
    VkFormat depthFormat{VK_FORMAT_UNDEFINED};

    bool operator==(const DepthState&) const noexcept = default;
};

struct ViewportState
{
    i32 x;
    i32 y;
    i32 width;
    i32 height;

    bool operator==(const ViewportState&) const noexcept = default;
};

struct GraphicsState
{
    RasterState         raster{};
    BlendState          blend{};
    DepthState          depth{};
    VkPrimitiveTopology topology{VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST};
    ViewportState       viewport{};

    bool operator==(const GraphicsState&) const noexcept = default;
};

struct RenderPipeline;
struct PipelineKey
{
    const RenderPipeline* renderPipeline;
    GraphicsState         graphicsState;
    bool                  operator==(const PipelineKey&) const noexcept = default;
};
} // namespace RED

namespace std
{
using namespace RED;
template <>
struct hash<RasterState>
{
    inline size_t operator()(const RasterState& state) const noexcept
    {
        size_t h1 = std::hash<VkPolygonMode>{}(state.polygonMode);
        size_t h2 = std::hash<VkCullModeFlags>{}(state.cullingMode);
        size_t h3 = std::hash<VkFrontFace>{}(state.frontFace);

        return h1 ^ (h2 << 1) ^ (h3 << 2);
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
struct hash<DepthState>
{
    inline size_t operator()(const DepthState& state) const noexcept
    {
        return std::hash<VkFormat>()(state.depthFormat);
    }
};

template <>
struct hash<ViewportState>
{
    inline size_t operator()(const ViewportState& state) const noexcept
    {
        size_t h1 = std::hash<i32>{}(state.x);
        size_t h2 = std::hash<i32>{}(state.y);
        size_t h3 = std::hash<i32>{}(state.width);
        size_t h4 = std::hash<i32>{}(state.height);

        return h1 ^ (h2 << 1) ^ (h3 << 2) ^ (h4 << 4);
    }
};

template <>
struct hash<GraphicsState>
{
    inline size_t operator()(const GraphicsState& state) const noexcept
    {
        size_t h1 = std::hash<RasterState>{}(state.raster);
        size_t h2 = std::hash<BlendState>{}(state.blend);
        size_t h3 = std::hash<DepthState>{}(state.depth);
        size_t h4 = std::hash<VkPrimitiveTopology>{}(state.topology);
        size_t h5 = std::hash<ViewportState>{}(state.viewport);

        return h1 ^ (h2 << 1) ^ (h3 << 2) ^ (h4 << 3) ^ (h5 << 4);
    }
};

template <>
struct hash<RED::PipelineKey>
{
    inline size_t operator()(const RED::PipelineKey& key) const noexcept
    {
        size_t h1 = hash<const RED::RenderPipeline*>{}(key.renderPipeline);
        size_t h2 = hash<RED::GraphicsState>{}(key.graphicsState);
        return h1 ^ (h2 << 1);
    };
};
} // namespace std
