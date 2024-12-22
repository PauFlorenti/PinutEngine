#include "stdafx.h"

#include "render_device/bufferDescriptor.h"
#include "render_device/device.h"
#include "render_device/drawCall.h"
#include "render_device/renderPipeline.h"

#include "src/renderer/pipelines/draw_texture_pipeline.h"
#include "src/renderer/stages/presentStage.h"

namespace Pinut
{
PresentStage::PresentStage() = default;

void PresentStage::Execute(RED::Device*               device,
                           const InputParameters&     parameters,
                           const RED::GPUTextureView& outputTarget)
{
    assert(device && "Device is nullptr");

    device->TransitionImageLayout(parameters.offscreenTexture,
                                  0,
                                  VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT,
                                  VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                  VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                                  VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                                  {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1});

    VkRenderingAttachmentInfo attachment{VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO};
    attachment.imageView   = parameters.swapchainImageView;
    attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    attachment.loadOp      = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachment.storeOp     = VK_ATTACHMENT_STORE_OP_STORE;
    attachment.clearValue  = {0.0f, 0.f, 0.f, 0.f};

    RED::ViewportState viewport{};
    viewport.x      = parameters.viewport.x;
    viewport.y      = parameters.viewport.y;
    viewport.width  = parameters.viewport.width;
    viewport.height = parameters.viewport.height;

    device->EnableRendering({parameters.viewport.x,
                             parameters.viewport.y,
                             static_cast<u32>(parameters.viewport.width),
                             static_cast<u32>(parameters.viewport.height)},
                            {attachment});

    RED::GraphicsState graphicsState{};
    graphicsState.viewport = std::move(viewport);
    graphicsState.depth    = {VK_FORMAT_UNDEFINED};

    device->SetGraphicsState(&graphicsState);
    device->SetRenderPipeline(&cDrawTexturePipeline);

    RED::DrawCall dc;
    dc.vertexBuffer = quadBuffer;
    dc.SetUniformTexture(parameters.offscreenTexture, RED::ShaderType::FRAGMENT, 0);

    device->SubmitDrawCalls({dc});

    device->DisableRendering();
}
} // namespace Pinut
