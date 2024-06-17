#pragma once

#include "vulkan/vulkan.h"
#include "vk_mem_alloc.h"

#include "descriptorSetManager.h"

struct GLFWwindow;

namespace Pinut
{
	class Mesh;
	class Device
	{
	public:
		Device() = default;
		~Device() = default;

		void OnCreate(const char *applicationName, const char *engineName, bool bCPUValidationEnabled, bool bGPUValidationEnabled, GLFWwindow *windowHandle);
		void OnDestroy();

		VkDevice GetDevice() const { return device; }
		VkPhysicalDevice GetPhysicalDevice() const { return physicalDevice; }
		VkSurfaceKHR GetSurface() const { return surface; }
		VmaAllocator GetAllocator() const { return allocator; }
		DescriptorSetManager &GetDescriptorSetManager() { return descriptorSetManager[frameIndex]; }

		VkQueue GetPresentQueue() const { return presentQueue; }
		VkQueue GetGraphicsQueue() const { return graphicsQueue; }
		VkQueue GetComputeQueue() const { return computeQueue; }
		uint32_t GetPresentQueueFamilyIndex() const { return presentQueueFamilyIndex; }
		uint32_t GetGraphicsQueueFamilyIndex() const { return graphicsQueueFamilyIndex; }
		uint32_t GetComputeQueueFamilyIndex() const { return computeQueueFamilyIndex; }

		uint32_t GetFrameIndex() const { return frameIndex; }
		void SetFrameIndex(const uint32_t idx) { frameIndex = idx; }

		void UploadMesh(const Mesh *mesh);

		VkCommandBuffer GetSingleUseCommandBuffer();
		void EndSingleUseCommandBuffer(VkCommandBuffer cmd) const;

	private:
		VkInstance instance;
		VkDevice device;
		VkPhysicalDevice physicalDevice;
		VkSurfaceKHR surface;

		VkQueue presentQueue;
		uint32_t presentQueueFamilyIndex;
		VkQueue graphicsQueue;
		uint32_t graphicsQueueFamilyIndex;
		VkQueue computeQueue;
		uint32_t computeQueueFamilyIndex;

		VkCommandPool singleUseCommandPool;

		uint32_t frameIndex{0};
		DescriptorSetManager descriptorSetManager[2];

		VkDebugUtilsMessengerEXT debugMessenger{nullptr};

		VmaAllocator allocator{nullptr};
	};
}
