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
struct PresentInputParameters
{
    RED::GPUBufferView   quadBuffer;
    RED::TextureResource offscreenTexture;
    ViewportData   viewport;
};

class PresentStage final : public Stage<PresentStage, PresentInputParameters>
{
  public:
    PresentStage();
    void Execute(RED::Device*               device,
                 const InputParameters&     parameters,
                 const RED::GPUTextureView& outputTarget);
};
} // namespace Pinut
