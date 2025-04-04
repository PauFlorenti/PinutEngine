#include "pch.hpp"

#include "render_device/bufferDescriptor.h"
#include "render_device/device.h"
#include "render_device/drawCall.h"
#include "render_device/renderPipeline.h"
#include "render_device/states.h"

#include "pinut/renderer/pipelines/draw_texture_pipeline.h"
#include "pinut/renderer/stages/presentStage.h"

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

    device->EnableRendering({parameters.viewport.x,
                             parameters.viewport.y,
                             static_cast<u32>(parameters.viewport.width),
                             static_cast<u32>(parameters.viewport.height)},
                            {});

    RED::GraphicsState graphicsState{};
    graphicsState.viewport = {parameters.viewport.x,
                              parameters.viewport.y,
                              parameters.viewport.width,
                              parameters.viewport.height};
    graphicsState.depth    = {RED::TextureFormat::UNDEFINED};

    device->SetGraphicsState(&graphicsState);
    device->SetRenderPipeline(&cDrawTexturePipeline);

    RED::DrawCall dc;
    dc.vertexBuffer = parameters.quadBuffer;
    dc.SetUniformTexture({parameters.offscreenTexture}, RED::ShaderType::FRAGMENT, 0);

    device->SubmitDrawCalls({dc});

    device->DisableRendering();
}
} // namespace Pinut
