#include "stdafx.h"

#include "render_device/bufferDescriptor.h"
#include "render_device/device.h"
#include "render_device/drawCall.h"
#include "render_device/renderPipeline.h"

#include "src/renderer/stages/lightForwardStage.h"

namespace Pinut
{
extern std::unordered_map<std::string, RED::RenderPipeline> m_pipelines;

LightForwardStage::LightForwardStage() = default;

void LightForwardStage::Execute(RED::Device*               device,
                                const InputParameters&     parameters,
                                const RED::GPUTextureView& outputTarget)
{
    assert(device && "Device is nullptr");

    device->TransitionImageLayout(parameters.colorFrameBuffer,
                                  0,
                                  VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
                                  VK_IMAGE_LAYOUT_UNDEFINED,
                                  VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                  VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                                  VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                                  {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1});

    RED::FrameBuffer attachment{.textureView    = parameters.colorFrameBuffer,
                                .loadOperation  = RED::FrameBufferLoadOperation::CLEAR,
                                .storeOperation = RED::FrameBufferStoreOperation::STORE,
                                .clearColor     = {.0f, .0f, .0f, .0f}};

    auto depthAttachment = RED::FrameBuffer{.textureView    = parameters.depthFrameBuffer,
                                            .loadOperation  = RED::FrameBufferLoadOperation::LOAD,
                                            .storeOperation = RED::FrameBufferStoreOperation::STORE,
                                            .clearColor     = {1.0f, .0f}};

    device->EnableRendering({parameters.viewport.x,
                             parameters.viewport.y,
                             static_cast<u32>(parameters.viewport.width),
                             static_cast<u32>(parameters.viewport.height)},
                            {attachment},
                            &depthAttachment);

    RED::GraphicsState graphicsState{};
    graphicsState.viewport = {parameters.viewport.x,
                              parameters.viewport.y,
                              parameters.viewport.width,
                              parameters.viewport.height};
    graphicsState.depth    = {VK_FORMAT_D32_SFLOAT, VK_COMPARE_OP_GREATER_OR_EQUAL, false};

    device->SetGraphicsState(&graphicsState);
    device->SetRenderPipeline(&m_pipelines.at("forward"));

    device->SubmitDrawCalls(parameters.drawCalls);

    device->DisableRendering();
}
} // namespace Pinut
