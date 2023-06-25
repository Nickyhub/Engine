#pragma once
#include <vulkan/vulkan.h>
#include "containers/DArray.hpp"
#include "Defines.hpp"
#include "VulkanFramebuffer.hpp"

struct VulkanSwapchain {
	VkSwapchainKHR s_Handle= nullptr;
	VkSurfaceFormatKHR s_SurfaceFormat{};
	VkPresentModeKHR s_PresentMode{};
	VkExtent2D s_Extent;

	DArray<VulkanFramebuffer> s_Framebuffers;

	unsigned int s_CurrentSwapchainImageIndex = INVALID_ID;

	DArray<VkImage> s_Images;
	unsigned int s_ImageCount = 0;

	DArray<VkImageView> s_ImageViews;
	unsigned int s_ImageViewCount = 0;

	unsigned int s_Width = 0;
	unsigned int s_Height = 0;
};

class VulkanSwapchainUtils {
public:
	static bool Create(VulkanSwapchain* outSwapchain, unsigned int width, unsigned int height);
	static bool Recreate(VulkanSwapchain* outSwapchain, unsigned int width, unsigned int  height);
	static bool AcquireNextImage(VulkanSwapchain* outSwapchain);
	static bool Present(VulkanSwapchain* swapchain);
	static void Destroy(VulkanSwapchain* swapchain);
};