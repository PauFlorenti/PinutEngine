#pragma once

#include "render_device/buffer.h"
#include "render_device/texture.h"

#include "pinut/renderer/stages/stage.h"

namespace RED
{
class Device;
}
namespace Pinut
{
struct SkyboxInputParameters
{
    ViewportData         viewport;
    RED::TextureResource colorFrameBuffer;
    RED::TextureResource depthFrameBuffer;
    RED::DrawCall        drawCall;
};

class SkyboxStage final : public Stage<SkyboxStage, SkyboxInputParameters>
{
  public:
    SkyboxStage();
    void Execute(RED::Device*               device,
                 const InputParameters&     parameters,
                 const RED::GPUTextureView& outputTarget);

  private:
    RED::GPUBuffer skyboxModelBuffer;
};
} // namespace Pinut
