// Some comment

#include "stdafx.h"

#include "renderer.h"

#include "src/core/common.h"
#include "src/core/device.h"
#include "src/core/swapchain.h"
#include "src/core/texture.h"
#include "src/core/vulkan_utils.h"
#include "src/misc/camera.h"
#include "src/scene/mesh.h"

using namespace Pinut;

void Renderer::OnCreate(Device *inDevice, Swapchain *inSwapchain)
{
	device = inDevice;

	uint32_t numberCommandBufferPerBackbuffer = 8;
	commandBufferRing.OnCreate(device, 2, numberCommandBufferPerBackbuffer);

	forwardPass.OnCreate(device);
}

void Renderer::OnDestroy()
{
	forwardPass.OnDestroy();
	commandBufferRing.OnDestroy();
}

void Renderer::OnRender(Swapchain *swapchain, const Scene *scene)
{
	BeginFrame(swapchain);
	Render(swapchain, scene);
	EndFrame(swapchain);
}

void Renderer::OnCreateDisplay(Swapchain *swapchain, uint32_t inWidth, uint32_t inHeight)
{
	width = inWidth;
	height = inHeight;

	viewport.x = 0.0f;
	viewport.y = static_cast<float>(height);
	viewport.width = static_cast<float>(width);
	viewport.height = -static_cast<float>(height);
	// viewport.minDepth = 0.0f;
	// viewport.maxDepth = 1.0f;

	scissor.extent.width = width;
	scissor.extent.height = height;
	scissor.offset = {0, 0};

	forwardPass.OnCreateDisplayDependantResources(width, height);
}

void Renderer::OnUpdateDisplay(Swapchain *swapchain)
{
	// TODO update pipelines
}

void Renderer::OnDestroyDisplay()
{
	forwardPass.OnDestroyDisplayDependantResources();
}

void Renderer::BeginFrame(Swapchain *swapchain)
{
	// TODO This should also check for resize.
	swapchain->WaitForSwapchain();
	device->GetDescriptorSetManager().Clear();
}

void Renderer::Render(Swapchain *swapchain, const Scene *scene)
{
	commandBufferRing.OnBeginFrame();

	VkSemaphore imageAvailableSemaphore{VK_NULL_HANDLE};
	VkSemaphore renderFinishedSemaphore{VK_NULL_HANDLE};
	VkFence fence{VK_NULL_HANDLE};
	swapchain->GetSyncObjects(&imageAvailableSemaphore, &renderFinishedSemaphore, &fence);

	auto cmd = commandBufferRing.GetNewCommandBuffer();
	auto cmdBeginInfo = Pinut::vkinit::CommandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
	assert(vkBeginCommandBuffer(cmd, &cmdBeginInfo) == VK_SUCCESS);

	Texture::TransitionImageLayout(cmd, swapchain->GetCurrentImage(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	// Texture::TransitionImageLayout(cmd, forwardPass.GetDepth().GetImage(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

	auto attachment = vkinit::RenderingAttachmentInfo(
		swapchain->GetCurrentImageView(),
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		VK_ATTACHMENT_LOAD_OP_CLEAR,
		VK_ATTACHMENT_STORE_OP_STORE,
		{0.0f, 0.0f, 0.0f, 0.0f});

	// Depth buffer
	// auto depthAttachment = vkinit::RenderingAttachmentInfo(
	// 	forwardPass.GetDepth().GetImageView(),
	// 	VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
	// 	VK_ATTACHMENT_LOAD_OP_CLEAR,
	// 	VK_ATTACHMENT_STORE_OP_STORE,
	// 	{1.0, 0});

	auto renderingInfo = vkinit::RenderingInfo(
		1,
		&attachment,
		{{0, 0}, {width, height}});

	vkCmdBeginRendering(cmd, &renderingInfo);

	vkCmdSetViewport(cmd, 0, 1, &viewport);
	vkCmdSetScissor(cmd, 0, 1, &scissor);

	vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, forwardPass.GetPipeline());

	// Upload per frame data
	const auto camera = scene->GetCamera();
	PerFrameData perFrameData{};

	perFrameData.view = camera->GetView();
	perFrameData.projection = camera->GetProjection();
	auto view = glm::lookAt(glm::vec3(0.0f, 2.0f, -5.0f), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	auto proj = glm::perspective(glm::radians(45.0f), (float)width / height, 0.01f, 1000.0f);

	//forwardPass.UpdatePerFrameData(cmd, perFrameData);
	forwardPass.Draw(cmd, scene);

	vkCmdEndRendering(cmd);

	Texture::TransitionImageLayout(cmd, swapchain->GetCurrentImage(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

	vkEndCommandBuffer(cmd);

	VkPipelineStageFlags stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	VkSubmitInfo submitInfo{
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &imageAvailableSemaphore,
		.pWaitDstStageMask = &stage,
		.commandBufferCount = 1,
		.pCommandBuffers = &cmd,
		.signalSemaphoreCount = 1,
		.pSignalSemaphores = &renderFinishedSemaphore};

	assert(vkQueueSubmit(device->GetGraphicsQueue(), 1, &submitInfo, fence) == VK_SUCCESS);
}

void Renderer::EndFrame(Swapchain *swapchain)
{
	swapchain->Present();
}
