#include "VulkanSwapchain.hpp"
#include "VulkanRenderer.hpp"
#include "VulkanFramebuffer.hpp"

bool VulkanSwapchainUtils::Create(VulkanSwapchain* outSwapchain, unsigned int width, unsigned int height) {
	// Get local copy of the Vulkan Device
	VulkanDevice* device = &VulkanRenderer::m_VulkanData.s_Device;
	// Choose the swapchain surface format
	bool formatFound = false;

	for (unsigned int i = 0; i < device->s_SwapchainSupportInfo.s_FormatCount; i++) {
		if (device->s_SwapchainSupportInfo.s_Formats[i].format == VK_FORMAT_B8G8R8A8_SRGB
			&& device->s_SwapchainSupportInfo.s_Formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			outSwapchain->s_SurfaceFormat = device->s_SwapchainSupportInfo.s_Formats[i];
			EN_INFO("Swapchain format found.");
			formatFound = true;
			break;
		}
	}
	// Choose the first format that is available if no suitable format has been found
	if (!formatFound) {
		outSwapchain->s_SurfaceFormat = device->s_SwapchainSupportInfo.s_Formats[0];
		EN_WARN("Swapchain has chosen a format that is not optimal because the optimal format is not available.");
	}

	// Choose presentation mode
	// Set default
	VkPresentModeKHR mode = VK_PRESENT_MODE_FIFO_KHR;
	// Look for better mode
	for (unsigned int i = 0; i < device->s_SwapchainSupportInfo.s_PresentModeCount; i++) {
		mode = device->s_SwapchainSupportInfo.s_PresentModes[i];
		if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
			outSwapchain->s_PresentMode = mode;
		}
	}

	// Choose the swapchain extent
	// NOTE set up with surface capabilities as this custom way may lead to problems
	VkExtent2D swapExtent;
	swapExtent.height = height;
	swapExtent.width = width;
	outSwapchain->s_Height = height;
	outSwapchain->s_Width = width;
	outSwapchain->s_Extent = swapExtent;

	unsigned int imageCount = device->s_SwapchainSupportInfo.s_Capabilities.minImageCount + 1;
	if (device->s_SwapchainSupportInfo.s_Capabilities.maxImageCount > 0 && imageCount > device->s_SwapchainSupportInfo.s_Capabilities.maxImageCount) {
		imageCount = device->s_SwapchainSupportInfo.s_Capabilities.maxImageCount;
	}

	// Actually creating the swapchain
	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = VulkanRenderer::m_VulkanData.s_Surface;

	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = outSwapchain->s_SurfaceFormat.format;
	createInfo.imageColorSpace = outSwapchain->s_SurfaceFormat.colorSpace;
	createInfo.imageExtent = swapExtent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	// Determining the image sharing mode
	if (device->s_PresentQueueFamilyIndex != device->s_GraphicsQueueFamilyIndex) {
		unsigned int familyIndices[] = { device->s_PresentQueueFamilyIndex, device->s_GraphicsQueueFamilyIndex };
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
	createInfo.preTransform = device->s_SwapchainSupportInfo.s_Capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = mode;
	createInfo.clipped = VK_TRUE;
	// TODO when resizing provide the old swapchain
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	VK_CHECK(vkCreateSwapchainKHR(device->s_LogicalDevice, &createInfo, VulkanRenderer::m_VulkanData.s_Allocator, &outSwapchain->s_Handle));
	EN_INFO("Swapchain created with width/height: %d/%d", width, height);

	// Retrieving the swapchain image handles to render to them later
	vkGetSwapchainImagesKHR(device->s_LogicalDevice, outSwapchain->s_Handle, &outSwapchain->s_ImageCount, nullptr);
	outSwapchain->s_Images.Resize(imageCount);
	vkGetSwapchainImagesKHR(device->s_LogicalDevice, outSwapchain->s_Handle, &outSwapchain->s_ImageCount, outSwapchain->s_Images.GetData());

	// Create image views
	outSwapchain->s_ImageViews.Resize(imageCount);
	outSwapchain->s_ImageViewCount = imageCount;

	for (unsigned int i = 0; i < imageCount; i++) {
		VkImageViewCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = outSwapchain->s_Images[i];
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = outSwapchain->s_SurfaceFormat.format;

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

		VK_CHECK(vkCreateImageView(device->s_LogicalDevice, &createInfo, VulkanRenderer::m_VulkanData.s_Allocator, &outSwapchain->s_ImageViews[i]));
	}

	return true;
}

bool VulkanSwapchainUtils::AcquireNextImage(VulkanSwapchain* outSwapchain) {
	VulkanData* d = &VulkanRenderer::m_VulkanData;
	VkResult result = vkAcquireNextImageKHR(
		d->s_Device.s_LogicalDevice,
		outSwapchain->s_Handle,
		UINT64_MAX,
		d->s_ImageAvailableSemaphores[d->s_CurrentFrame],
		VK_NULL_HANDLE,
		&d->s_Swapchain.s_CurrentSwapchainImageIndex);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
		if (!Recreate(outSwapchain, d->s_FramebufferWidth, d->s_FramebufferHeight)) {
			EN_ERROR("Failed to recreate Swapchain.");
			return false;
		}
		return false;
	}
	return true;
}

bool VulkanSwapchainUtils::Recreate(VulkanSwapchain* outSwapchain, unsigned int width, unsigned int  height) {
	vkDeviceWaitIdle(VulkanRenderer::m_VulkanData.s_Device.s_LogicalDevice);
	Destroy(outSwapchain);
	if (!Create(outSwapchain, width, height)) {
		EN_ERROR("Failed to create swapchain while recreating the swapchain.");
		return false;
	}

	// Create framebuffers
	for (unsigned int i = 0; i < VulkanRenderer::m_VulkanData.s_Swapchain.s_ImageCount; i++) {
		if (!VulkanFramebufferUtils::Create(&outSwapchain->s_Framebuffers[i], i)) {
			EN_ERROR("Failed to create framebuffer for image index %d.", i);
			return false;
		}
	}
	return true;
}

bool VulkanSwapchainUtils::Present(VulkanSwapchain* swapchain) {
	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &VulkanRenderer::m_VulkanData.s_RenderFinishedSemaphores[VulkanRenderer::m_VulkanData.s_CurrentFrame];

	VkSwapchainKHR swapchains[] = { swapchain->s_Handle };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapchains;
	presentInfo.pImageIndices = &swapchain->s_CurrentSwapchainImageIndex;

	vkQueuePresentKHR(VulkanRenderer::m_VulkanData.s_Device.s_PresentQueue, &presentInfo);
	return true;
}

void VulkanSwapchainUtils::Destroy(VulkanSwapchain* swapchain) {
	VulkanData* d = &VulkanRenderer::m_VulkanData;

	// Destroy framebuffers
	for (unsigned int i = 0; i < swapchain->s_ImageCount; i++) {
		if (swapchain->s_Framebuffers[i].s_Handle) {
			vkDestroyFramebuffer(d->s_Device.s_LogicalDevice, swapchain->s_Framebuffers[i].s_Handle, d->s_Allocator);
		}
	}

	// Destroy image views before destroying the swapchain itself
	for (unsigned int i = 0; i < swapchain->s_ImageViewCount; i++) {
		vkDestroyImageView(d->s_Device.s_LogicalDevice, swapchain->s_ImageViews[i], d->s_Allocator);
	}
	swapchain->s_Height = 0;
	swapchain->s_Width = 0;
	vkDestroySwapchainKHR(d->s_Device.s_LogicalDevice, swapchain->s_Handle, d->s_Allocator);
	swapchain->s_Handle = 0;
}