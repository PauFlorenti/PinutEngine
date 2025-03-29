#pragma once

namespace RED
{
class Device;
class GPUTextureView;
struct RenderPipeline;
} // namespace RED
namespace Pinut
{
template <class StageInstance, typename StageParameters>
class Stage
{
  public:
    using InputParameters = StageParameters;

    virtual ~Stage() = default;

    void Init(const RED::Device& device) { static_cast<StageInstance*>(this)->Init(device); }

    void Execute(RED::Device*               device,
                 const InputParameters&     parameters,
                 const RED::GPUTextureView& outputTarget)
    {
        static_cast<StageInstance*>(this)->Execute(device, parameters, outputTarget);
    };

  protected:
    RED::RenderPipeline* renderPipeline{nullptr};
};
} // namespace Pinut
