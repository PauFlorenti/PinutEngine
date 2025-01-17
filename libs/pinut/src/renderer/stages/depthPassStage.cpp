#include "stdafx.h"

#include "render_device/device.h"
#include "render_device/drawCall.h"
#include "render_device/renderPipeline.h"

#include "src/renderer/stages/depthPassStage.h"

namespace Pinut
{
extern std::unordered_map<std::string, RED::RenderPipeline> m_pipelines;

DepthPassStage::DepthPassStage() = default;

void DepthPassStage::Execute(RED::Device*               device,
                             const InputParameters&     parameters,
                             const RED::GPUTextureView& outputTarget)
{
    assert(device && "Device is nullptr");

    auto depthAttachment = RED::FrameBuffer{.textureView    = parameters.depthFrameBuffer,
                                            .loadOperation  = RED::FrameBufferLoadOperation::CLEAR,
                                            .storeOperation = RED::FrameBufferStoreOperation::STORE,
                                            .clearColor     = {1.0f, .0f}};

    device->EnableRendering({parameters.viewport.x,
                             parameters.viewport.y,
                             static_cast<u32>(parameters.viewport.width),
                             static_cast<u32>(parameters.viewport.height)},
                            {},
                            &depthAttachment);

    RED::GraphicsState graphicsState{};
    graphicsState.viewport = {parameters.viewport.x,
                              parameters.viewport.y,
                              parameters.viewport.width,
                              parameters.viewport.height};
    graphicsState.depth    = {VK_FORMAT_D32_SFLOAT, VK_COMPARE_OP_LESS_OR_EQUAL, true};

    device->SetGraphicsState(&graphicsState);
    device->SetRenderPipeline(&m_pipelines.at("depth_pass"));

    device->SubmitDrawCalls(parameters.drawCalls);

    device->DisableRendering();
}
} // namespace Pinut
