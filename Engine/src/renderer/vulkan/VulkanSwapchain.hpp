#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include "Defines.hpp"

#include "VulkanDevice.hpp"
#include "VulkanSyncObjects.hpp"
#include "VulkanRenderpass.hpp"
#include "VulkanFramebuffer.hpp"

struct VulkanSwapchainConfig {
	unsigned int s_Width, s_Height;
	unsigned int s_FramesInFlight;
	const VulkanDevice& s_Device;
	const VkAllocationCallbacks& s_Allocator;
};

class VulkanSwapchain {
public:
	VulkanSwapchain() = delete;
	VulkanSwapchain(const VulkanSwapchainConfig& config);

	bool recreate(const VulkanSwapchainConfig& config, const VulkanRenderpass& renderpass);
	bool acquireNextImage(const VulkanRenderpass& renderpass);
	bool present();

	void createFramebuffers(const VulkanRenderpass& renderpass);
	void createDepthImage(const VulkanImageConfig& config);

	~VulkanSwapchain();
private:
	bool create(const VulkanSwapchainConfig& config);
	void destroy();
public:
	unsigned int m_Width = 0;
	unsigned int m_Height = 0;
	unsigned int m_ImageCount = 0;
	unsigned int m_CurrentFrame = 0; 
	unsigned int m_CurrentSwapchainImageIndex = INVALID_ID;

	const unsigned int& m_FramesInFlight;
	const VkAllocationCallbacks& m_Allocator;
	const VulkanDevice& m_Device;
	VkExtent2D m_Extent;
	VkSurfaceFormatKHR m_SurfaceFormat{};
	std::vector<VkImageView> m_ImageViews;
	std::vector<VulkanFramebuffer*> m_Framebuffers;

	VulkanImage* m_DepthImage;
	
	// Sync Objects
	std::vector<VulkanSemaphore*> m_ImageAvailableSemaphores{};
	std::vector<VulkanSemaphore*> m_RenderFinishedSemaphores{};
	std::vector<VulkanFence*> m_InFlightFences{};

private:
	VkSwapchainKHR m_Handle = nullptr;
	VkPresentModeKHR m_PresentMode{};
	std::vector<VkImage> m_Images;

	unsigned int m_ImageViewCount = 0;
};