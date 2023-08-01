#pragma once

#include <vulkan/vulkan.h>
#include <containers/DArray.hpp>

struct VulkanImage {
	VkImage s_Handle;
	VkImageView s_View;
	VkDeviceMemory s_Memory;
	int s_Width = 0;
	int s_Height = 0;
	int s_Channels = 0;
};


class VulkanImageUtils {
public:
	static bool Create(VulkanImage& outImage);
	static void Destroy(VulkanImage& image);
	static VkFormat FindSupportedFormat(DArray<VkFormat> candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
	static VkFormat FindDepthFormat();
	static bool HasStencilComponent(VkFormat format) { return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT; }
	static bool CreateImage(uint32_t width,
		uint32_t height,
		VkFormat format,
		VkImageTiling tiling,
		VkImageUsageFlags usage,
		VkMemoryPropertyFlags properties,
		VkImage& image, VkDeviceMemory& imageMemory);
	static bool CreateImageView(VulkanImage& outImage, VkFormat format, VkImageAspectFlags aspectFlags);
	static bool CreateTextureSampler(VkSampler& outSampler);
	static bool CreateDepthResources(VulkanImage& depthImage);
	static void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
	static void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
private:
};