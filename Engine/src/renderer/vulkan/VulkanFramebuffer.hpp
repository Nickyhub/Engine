#pragma once
#include <vulkan/vulkan.h>
#include "VulkanRenderpass.hpp"
#include "VulkanImage.hpp"

class VulkanSwapchain;

struct VulkanFramebufferConfig {
	unsigned int s_Width;
	unsigned int s_Height;
	unsigned int imageIndex;

	const VkAllocationCallbacks& s_Allocator;
	const VulkanDevice& s_Device;
	const VulkanImage& s_DepthImage;
	const VulkanRenderpass& s_Renderpass;
	const VulkanSwapchain& s_Swapchain;
};

class VulkanFramebuffer {
public:
	VulkanFramebuffer() = delete;
	VulkanFramebuffer(const VulkanFramebufferConfig& config);
	~VulkanFramebuffer();
public:
	VkFramebuffer m_Handle;
private:
	const VkAllocationCallbacks& m_Allocator;
	const VulkanDevice& m_Device;
};