#include "core/Logger.hpp"

#include "VulkanSwapchain.hpp"
#include "VulkanUtils.hpp"

VulkanSwapchain::VulkanSwapchain(const VulkanSwapchainConfig& config) :
m_Device(config.s_Device), m_Allocator(config.s_Allocator) {
	create(config);
}

bool VulkanSwapchain::create(const VulkanSwapchainConfig& config) {
	// Choose the swapchain surface format
	bool formatFound = false;

	for (unsigned int i = 0; i < m_Device.m_SwapchainSupportInfo.s_FormatCount; i++) {
		if (m_Device.m_SwapchainSupportInfo.s_Formats[i].format == VK_FORMAT_B8G8R8A8_SRGB
			&& m_Device.m_SwapchainSupportInfo.s_Formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			m_SurfaceFormat = m_Device.m_SwapchainSupportInfo.s_Formats[i];
			EN_INFO("Swapchain format found.");
			formatFound = true;
			break;
		}
	}
	// Choose the first format that is available if no suitable format has been found
	if (!formatFound) {
		m_SurfaceFormat = m_Device.m_SwapchainSupportInfo.s_Formats[0];
		EN_WARN("Swapchain has chosen a format that is not optimal because the optimal format is not available.");
	}

	// Choose presentation mode
	// Set default
	VkPresentModeKHR mode = VK_PRESENT_MODE_FIFO_KHR;
	// Look for better mode
	for (unsigned int i = 0; i < m_Device.m_SwapchainSupportInfo.s_PresentModeCount; i++) {
		mode = m_Device.m_SwapchainSupportInfo.s_PresentModes[i];
		if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
			m_PresentMode = mode;
		}
	}

	// Choose the swapchain extent
	// NOTE set up with surface capabilities as this custom way may lead to problems
	VkExtent2D swapExtent;
	swapExtent.height = config.s_Height;
	swapExtent.width = config.s_Width;
	m_Height = config.s_Height;
	m_Width = config.s_Width;
	m_Extent = swapExtent;

	unsigned int imageCount = m_Device.m_SwapchainSupportInfo.s_Capabilities.minImageCount + 1;
	if (m_Device.m_SwapchainSupportInfo.s_Capabilities.maxImageCount > 0 &&
		imageCount > m_Device.m_SwapchainSupportInfo.s_Capabilities.maxImageCount) {
		imageCount = m_Device.m_SwapchainSupportInfo.s_Capabilities.maxImageCount;
	}

	// Actually creating the swapchain
	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = m_Device.m_Surface;

	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = m_SurfaceFormat.format;
	createInfo.imageColorSpace = m_SurfaceFormat.colorSpace;
	createInfo.imageExtent = swapExtent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	// Determining the image sharing mode
	if (m_Device.m_PresentQueueFamilyIndex != m_Device.m_GraphicsQueueFamilyIndex) {
		unsigned int familyIndices[] = { m_Device.m_PresentQueueFamilyIndex, m_Device.m_GraphicsQueueFamilyIndex };
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = familyIndices;
		EN_TRACE("Vulkan swapchain imageSharing mode is concurrent.");
	}
	else {
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0;
		createInfo.pQueueFamilyIndices = nullptr;
		EN_TRACE("Vulkan swapchain imageSharing mode is exclusive.");
	}
	createInfo.preTransform = m_Device.m_SwapchainSupportInfo.s_Capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = mode;
	createInfo.clipped = VK_TRUE;
	// TODO when resizing provide the old swapchain
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	VK_CHECK(vkCreateSwapchainKHR(m_Device.m_LogicalDevice, &createInfo, &m_Allocator, &m_Handle));
	EN_INFO("Swapchain created with width/height: %d/%d", config.s_Width, config.s_Height);

	// Retrieving the swapchain image handles to render to them later
	vkGetSwapchainImagesKHR(m_Device.m_LogicalDevice, m_Handle, &m_ImageCount, nullptr);
	m_Images.resize(imageCount);
	vkGetSwapchainImagesKHR(m_Device.m_LogicalDevice, m_Handle, &m_ImageCount, m_Images.data());

	// Create image views
	m_ImageViews.resize(imageCount);
	m_ImageViewCount = imageCount;

	for (unsigned int i = 0; i < imageCount; i++) {
		// TODO call VulkanImageUtils::CreateImageView. Somehow this should be cleaned up anyway then.
		VkImageViewCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = m_Images[i];
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = m_SurfaceFormat.format;

		// Swizzle color channels (maybe create red texture. Test around with this. For now set to default
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		// Specify the images purpose. IN this case the are color targets without mipmapping or layers
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		VK_CHECK(vkCreateImageView(m_Device.m_LogicalDevice,
									&createInfo,
									&m_Allocator, &m_ImageViews[i]));
	}

	return true;
}

void VulkanSwapchain::createSyncObjects(unsigned int framesInFlight) {
	m_ImageAvailableSemaphores.resize(framesInFlight);
	m_RenderFinishedSemaphores.resize(framesInFlight);
	m_InFlightFences.resize(framesInFlight);
	for (unsigned int i = 0; i < framesInFlight; i++) {
		m_ImageAvailableSemaphores[i] = new VulkanSemaphore(m_Device, m_Allocator);
		m_RenderFinishedSemaphores[i] = new VulkanSemaphore(m_Device, m_Allocator);
		m_InFlightFences[i] = new VulkanFence(m_Device, m_Allocator);
	}
}

bool VulkanSwapchain::acquireNextImage() {
	VkResult result = vkAcquireNextImageKHR(
		m_Device.m_LogicalDevice,
		m_Handle,
		UINT64_MAX,
		m_ImageAvailableSemaphores[m_CurrentFrame]->m_Handle,
		VK_NULL_HANDLE,
		&m_CurrentSwapchainImageIndex);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
		if (!recreate({m_Width, m_Height, m_Device, m_Allocator})) {
			EN_ERROR("Failed to recreate Swapchain.");
			return false;
		}
		return false;
	}
	return true;
}

bool VulkanSwapchain::recreate(const VulkanSwapchainConfig& config) {
	vkDeviceWaitIdle(m_Device.m_LogicalDevice);
	destroy();
	if (!create(config)) {
		EN_ERROR("Failed to create swapchain while recreating the swapchain.");
		return false;
	}

	// Destroy old framebuffers
	for (unsigned int i = 0; i < m_ImageCount; i++) {

	}

	// Create framebuffers
	for (unsigned int i = 0; i < m_ImageCount; i++) {
		
	}
	return true;
}

bool VulkanSwapchain::present() {
	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &m_RenderFinishedSemaphores[m_CurrentFrame]->m_Handle;

	VkSwapchainKHR swapchains[] = { m_Handle };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapchains;
	presentInfo.pImageIndices = &m_CurrentSwapchainImageIndex;

	vkQueuePresentKHR(m_Device.m_PresentQueue, &presentInfo);
	return true;
}

void VulkanSwapchain::destroy() {
	// Destroy image views before destroying the swapchain itself
	for (unsigned int i = 0; i < m_ImageViewCount; i++) {
		vkDestroyImageView(m_Device.m_LogicalDevice, m_ImageViews[i], &m_Allocator);
	}
	m_Height = 0;
	m_Width = 0;
	vkDestroySwapchainKHR(m_Device.m_LogicalDevice, m_Handle, &m_Allocator);
	m_Handle = 0;

	for (unsigned int i = 0; i < m_ImageAvailableSemaphores.size(); i++) {
		delete m_ImageAvailableSemaphores[i];
		delete m_RenderFinishedSemaphores[i];
		delete m_InFlightFences[i];
	}
	EN_INFO("Vulkan swapchain destroyed.");
}

VulkanSwapchain::~VulkanSwapchain() {
	destroy();
}