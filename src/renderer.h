#pragma once

#include "src/core/commandBufferRing.h"
#include "src/pipelines/forward.h"
#include "src/scene/scene.h"

namespace Pinut
{
    class Device;
    class Swapchain;
}

using namespace Pinut;

class Renderer
{
public:
    Renderer() = default;
    Renderer(const Renderer &) = delete;
    Renderer &operator=(const Renderer &) = delete;

    void OnCreate(Device *device, Swapchain *swapchain);
    void OnDestroy();

    void OnRender(Swapchain *swapchain, const Scene *scene);
    void OnCreateDisplay(Swapchain *swapchain, uint32_t width, uint32_t height);
    void OnUpdateDisplay(Swapchain *swapchain);
    void OnDestroyDisplay();

private:
    void BeginFrame(Swapchain* swapchain);
    void Render(Swapchain* swapchain, const Scene* scene);
    void EndFrame(Swapchain* swapchain);

    Device *device{nullptr};

    uint32_t width;
    uint32_t height;

    VkRect2D scissor;
    VkViewport viewport;

    CommandBufferRing commandBufferRing;

    // Passes
    ForwardPass forwardPass;

    // Renderings
    // VkRenderingInfo forwardRenderingInfo;
};
