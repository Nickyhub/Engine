#pragma once

#include <vulkan/vulkan.h>

class VulkanSyncObjects {
public:
	static bool CreateVkSemaphore(VkSemaphore* outSemaphore);
	static bool CreateVkFence(VkFence* outFence);

	static void DestroyVkSemaphore(VkSemaphore* semaphore);
	static void DestroyVkFence(VkFence* fence);
};