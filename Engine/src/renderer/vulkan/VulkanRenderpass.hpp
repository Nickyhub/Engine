#pragma once
#include <vulkan/vulkan.h>
#include "containers/DArray.hpp"
#include "VulkanCommandbuffer.hpp"

struct VulkanRenderpass {
	VkRenderPass s_Handle;
};

class VulkanRenderpassUtils {
public:
	static bool Create(VulkanRenderpass* outRenderpass);
	static bool Begin(VulkanRenderpass* renderpass, unsigned int imageIndex, VulkanCommandbuffer* commandBuffer);
	static bool End(VulkanRenderpass* renderpass, unsigned int imageIndex, VulkanCommandbuffer* commandBuffer);
	static void Destroy(VulkanRenderpass* renderpass);
};