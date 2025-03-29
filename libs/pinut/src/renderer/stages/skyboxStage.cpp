#include "pch.hpp"

#include "render_device/bufferDescriptor.h"
#include "render_device/device.h"
#include "render_device/drawCall.h"
#include "render_device/renderPipeline.h"
#include "render_device/states.h"
#include "render_device/textureFormat.h"

#include "pinut/renderer/pipelines/skybox_pipeline.h"
#include "pinut/renderer/stages/skyboxStage.h"

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
    graphicsState.viewport               = {parameters.viewport.x,
                                            parameters.viewport.y,
                                            parameters.viewport.width,
                                            parameters.viewport.height};
    graphicsState.depth.depthFormat      = RED::TextureFormat::D32_SFLOAT;
    graphicsState.depth.compareOperation = VK_COMPARE_OP_LESS_OR_EQUAL;

    device->SetGraphicsState(&graphicsState);
    device->SetRenderPipeline(&cSkyboxPipeline);

    device->SubmitDrawCalls({parameters.drawCall});

    device->DisableRendering();
}
} // namespace Pinut
