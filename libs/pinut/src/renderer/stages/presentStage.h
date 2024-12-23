#pragma once

#include "render_device/buffer.h"
#include "render_device/states.h"
#include "render_device/texture.h"

#include "src/renderer/stages/stage.h"

namespace RED
{
class Device;
}
namespace Pinut
{
struct PresentInputParameters
{
    RED::GPUBufferView   quadBuffer;
    RED::TextureResource offscreenTexture;
    RED::ViewportState   viewport;
};

class PresentStage final : public Stage<PresentStage, PresentInputParameters>
{
  public:
    PresentStage();
    void Execute(RED::Device*               device,
                 const InputParameters&     parameters,
                 const RED::GPUTextureView& outputTarget);

  private:
    RED::GPUBuffer quadBuffer;
};
} // namespace Pinut
