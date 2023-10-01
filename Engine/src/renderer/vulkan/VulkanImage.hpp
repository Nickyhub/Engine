#pragma once

#include <vulkan/vulkan.h>
#include <vector>

#include "VulkanDevice.hpp"

struct VulkanImageConfig {
	int s_Width;
	int s_Height;
	VkFormat s_Format;
	VkImageTiling s_Tiling;
	VkImageUsageFlags s_Usage;
	VkMemoryPropertyFlags s_Properties;

	const VulkanDevice& s_Device;
	const VkAllocationCallbacks& s_Allocator;
};

class VulkanImage {
public:
	VulkanImage() = delete;
	VulkanImage(const VulkanImageConfig& config);
	~VulkanImage();
private:
	VkFormat findSupportedFormat(std::vector<VkFormat> candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
	bool hasStencilComponent(VkFormat format) { return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT; }
	bool createImage(const VulkanImageConfig& config);
	bool createImageView(VkFormat format, VkImageAspectFlags aspectFlags);
	bool createTextureSampler(VkSampler& outSampler);
	bool createDepthImage(const VulkanImageConfig& config);
	void transitionImageLayout(VkImage image,
							   VkFormat format,
							   VkImageLayout oldLayout,
							   VkImageLayout newLayout);
	void copyBufferToImage(const VkBuffer& buffer,
						   uint32_t width,
						   uint32_t height);
public:
	VkSampler m_Sampler{};
	VkImageView m_View{};
private:
	const VulkanDevice& m_Device;
	const VkAllocationCallbacks& m_Allocator;

	VkImage m_Handle{};
	VkDeviceMemory m_Memory{};
	int m_Width = 0;
	int m_Height = 0;
	int m_Channels = 0;
};