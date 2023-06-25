#pragma once
#include <vulkan/vulkan.h>

struct VulkanFramebuffer {
	VkFramebuffer s_Handle = nullptr;
};

class VulkanFramebufferUtils {
public:
	static bool Create(VulkanFramebuffer* outFrambuffer, unsigned int imageIndex);

	static void Destroy(VulkanFramebuffer* frambuffer);
};