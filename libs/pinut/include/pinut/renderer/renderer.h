#pragma once

#include "pinut/renderer/offscreenState.h"
#include "pinut/renderer/stages/depthPassStage.h"
#include "pinut/renderer/stages/lightForwardStage.h"
#include "pinut/renderer/stages/presentStage.h"
#include "pinut/renderer/stages/skyboxStage.h"

struct GLFWwindow;
namespace RED
{
class Device;
} // namespace RED
namespace Pinut
{
class PinutImGUI;

struct SwapchainInfo;

class Renderer final
{
  public:
    explicit Renderer(std::shared_ptr<RED::Device> device,
                      SwapchainInfo*               swapchain,
                      std::unique_ptr<PinutImGUI>  imgui = nullptr);
    ~Renderer();

    void Render(entt::registry& registry, const ViewportData& viewportData, bool resized);

  private:
    void Update(entt::registry& registry, const ViewportData& viewportData, bool resized);

    std::shared_ptr<RED::Device> m_device{nullptr};
    SwapchainInfo*               m_swapchain{nullptr};

    entt::registry m_rendererRegistry;

    DepthPassStage    m_depthPassStage;
    OffscreenState    m_offscreenState;
    PresentStage      m_presentStage;
    LightForwardStage m_lightForwardStage;
    SkyboxStage       m_skyboxStage;

#ifdef _DEBUG
    std::unique_ptr<PinutImGUI> m_imgui{nullptr};
#endif
};
} // namespace Pinut
