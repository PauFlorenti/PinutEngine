#pragma once

#include "render_device/drawCall.h"
#include "render_device/states.h"

#include "src/renderer/stages/stage.h"

namespace RED
{
class Device;
}
namespace Pinut
{
struct DepthPassInputParameters
{
    ViewportData               viewport;
    RED::TextureResource       depthFrameBuffer;
    std::vector<RED::DrawCall> drawCalls;
};
class DepthPassStage final : public Stage<DepthPassStage, DepthPassInputParameters>
{
  public:
    DepthPassStage();
    void Execute(RED::Device*               device,
                 const InputParameters&     parameters,
                 const RED::GPUTextureView& outputTarget);
};
} // namespace Pinut
