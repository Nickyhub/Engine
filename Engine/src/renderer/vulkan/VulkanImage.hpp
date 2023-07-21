#pragma once

#include <vulkan/vulkan.h>

struct VulkanImage {
	VkImage s_Handle;
	VkImageView s_View;
	VkSampler s_Sampler;
	VkDeviceMemory s_Memory;
	int s_Width = 0;
	int s_Height = 0;
	int s_Channels = 0;
};


class VulkanImageUtils {
public:
	static bool Create(VulkanImage& outImage);
	static void Destroy(VulkanImage& image);
private:
	static bool CreateImage(uint32_t width,
		uint32_t height,
		VkFormat format,
		VkImageTiling tiling,
		VkImageUsageFlags usage,
		VkMemoryPropertyFlags properties,
		VkImage& image, VkDeviceMemory& imageMemory);
	static bool CreateImageView(VulkanImage& outImage);
	static bool CreateTextureSampler(VulkanImage& outImage);
	static void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
	static void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
};