#pragma once

#include <entt/entt.hpp>
#include <external/vk-bootstrap/src/VkBootstrap.h>

#include "render_device/device.h"
#include "render_device/renderPipeline.h"
#include "render_device/texture.h"

#include "src/components/renderComponent.h"
#include "src/components/transformComponent.h"
#include "src/renderer/offscreenState.h"
#include "src/renderer/stages/lightForwardStage.h"
#include "src/renderer/stages/presentStage.h"

struct GLFWwindow;
namespace RED
{
class Device;
} // namespace RED
namespace Pinut
{
struct SwapchainInfo;

class Renderer final
{
  public:
    explicit Renderer(std::shared_ptr<RED::Device> device, SwapchainInfo* swapchain);
    ~Renderer();

    void Render(entt::registry& registry, const ViewportData& viewportData, bool resized);

  private:
    void Update(entt::registry& registry, const ViewportData& viewportData, bool resized);

    std::shared_ptr<RED::Device> m_device{nullptr};
    SwapchainInfo*               m_swapchain{nullptr};

    entt::registry m_rendererRegistry;

    OffscreenState    m_offscreenState;
    PresentStage      m_presentStage;
    LightForwardStage m_lightForwardStage;
};
} // namespace Pinut
