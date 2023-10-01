#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include "Defines.hpp"

#include "VulkanDevice.hpp"
#include "VulkanSyncObjects.hpp"

struct VulkanSwapchainConfig {
	unsigned int s_Width, s_Height;
	const VulkanDevice& s_Device;
	const VkAllocationCallbacks& s_Allocator;
};

class VulkanSwapchain {
public:
	VulkanSwapchain() = delete;
	VulkanSwapchain(const VulkanSwapchainConfig& config);

	void createSyncObjects(unsigned int framesInFlight);

	bool recreate(const VulkanSwapchainConfig& config);
	bool acquireNextImage();
	bool present();

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

	const VkAllocationCallbacks& m_Allocator;
	const VulkanDevice& m_Device;
	VkExtent2D m_Extent;
	VkSurfaceFormatKHR m_SurfaceFormat{};
	std::vector<VkImageView> m_ImageViews;

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