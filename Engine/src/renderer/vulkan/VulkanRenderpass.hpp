#pragma once
#include <vulkan/vulkan.h>

#include <vector>
#include "VulkanCommandbuffer.hpp"

class VulkanFramebuffer;

class VulkanRenderpass {
public:
	VulkanRenderpass() = delete;
	VulkanRenderpass(const VulkanDevice& device, VkFormat colorFormat, const VkAllocationCallbacks& allocator);
	~VulkanRenderpass();

	bool begin(unsigned int imageIndex, VulkanCommandbuffer* commandBuffer, VkExtent2D extent, const std::vector<VulkanFramebuffer*>& framebuffers);
	bool end(unsigned int imageIndex, VulkanCommandbuffer* commandBuffer);

public:
	VkRenderPass m_Handle{};
private:
	const VulkanDevice& m_Device;
	const VkAllocationCallbacks& m_Allocator;
};