#include "stdafx.h"

#include "render_device/bufferDescriptor.h"
#include "render_device/device.h"
#include "render_device/drawCall.h"
#include "render_device/renderPipeline.h"

#include "src/renderer/pipelines/skybox_pipeline.h"
#include "src/renderer/stages/skyboxStage.h"

namespace Pinut
{
SkyboxStage::SkyboxStage() = default;

void SkyboxStage::Execute(RED::Device*               device,
                          const InputParameters&     parameters,
                          const RED::GPUTextureView& outputTarget)
{
    assert(device && "Device is nullptr");

    RED::FrameBuffer attachment{.textureView    = parameters.colorFrameBuffer,
                                .loadOperation  = RED::FrameBufferLoadOperation::LOAD,
                                .storeOperation = RED::FrameBufferStoreOperation::STORE,
                                .clearColor     = {.0f, .0f, .0f, .0f}};

    RED::FrameBuffer depthAttachment{.textureView    = parameters.depthFrameBuffer,
                                     .loadOperation  = RED::FrameBufferLoadOperation::LOAD,
                                     .storeOperation = RED::FrameBufferStoreOperation::STORE,
                                     .clearColor     = {}};

    device->EnableRendering({parameters.viewport.x,
                             parameters.viewport.y,
                             static_cast<u32>(parameters.viewport.width),
                             static_cast<u32>(parameters.viewport.height)},
                            {std::move(attachment)},
                            &depthAttachment);

    RED::GraphicsState graphicsState{};
    graphicsState.viewport               = parameters.viewport;
    graphicsState.depth.depthFormat      = VK_FORMAT_D32_SFLOAT;
    graphicsState.depth.compareOperation = VK_COMPARE_OP_LESS_OR_EQUAL;

    device->SetGraphicsState(&graphicsState);
    device->SetRenderPipeline(&cSkyboxPipeline);

    device->SubmitDrawCalls({parameters.drawCall});

    device->DisableRendering();
}
} // namespace Pinut
