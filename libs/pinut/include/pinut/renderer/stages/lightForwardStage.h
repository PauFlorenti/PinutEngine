#pragma once

#include "render_device/buffer.h"
#include "render_device/drawCall.h"
#include "render_device/texture.h"

#include "pinut/renderer/stages/stage.h"

namespace RED
{
class Device;
}
namespace Pinut
{
struct DrawOpaqueInputParameters
{
    ViewportData               viewport;
    RED::TextureResource       colorFrameBuffer;
    RED::TextureResource       depthFrameBuffer;
    std::vector<RED::DrawCall> drawCalls;
};

class LightForwardStage final : public Stage<LightForwardStage, DrawOpaqueInputParameters>
{
  public:
    LightForwardStage();
    void Execute(RED::Device*               device,
                 const InputParameters&     parameters,
                 const RED::GPUTextureView& outputTarget);
};
} // namespace Pinut
