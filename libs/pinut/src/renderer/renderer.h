#pragma once

#include <entt/entt.hpp>
#include <external/vk-bootstrap/src/VkBootstrap.h>

#include "render_device/device.h"
#include "render_device/offscreenState.h"
#include "render_device/renderPipeline.h"
#include "render_device/texture.h"

#include "src/components/renderComponent.h"
#include "src/components/transformComponent.h"

struct GLFWwindow;
namespace RED
{
class Device;
} // namespace RED
namespace Pinut
{
struct SwapchainInfo;
struct ViewportData
{
    i32       x{0};
    i32       y{0};
    i32       width{0};
    i32       height{0};
    glm::mat4 view{glm::mat4(1.0f)};
    glm::mat4 projection{glm::mat4(1.0f)};
    glm::vec3 cameraPosition{glm::vec3(0.0f)};
};
class Renderer final
{
  public:
    explicit Renderer(std::shared_ptr<RED::Device> device, SwapchainInfo* swapchain);
    ~Renderer();

    void Update(entt::registry& registry, const ViewportData& viewportData, bool resized);
    void Render(entt::registry& registry, const ViewportData& viewportData);

  private:
    std::shared_ptr<RED::Device> m_device{nullptr};
    SwapchainInfo*               m_swapchain{nullptr};

    RED::OffscreenState m_offscreenState;

    std::unordered_map<std::string, RED::RenderPipeline> m_pipelines;
    entt::registry                                       m_rendererRegistry;
};
} // namespace Pinut
