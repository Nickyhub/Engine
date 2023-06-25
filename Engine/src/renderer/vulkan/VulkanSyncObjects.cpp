#include "VulkanSyncObjects.hpp"
#include "VulkanRenderer.hpp"

bool VulkanSyncObjects::CreateVkSemaphore(VkSemaphore* outSemaphore) {
	VkSemaphoreCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	VulkanData* d = &VulkanRenderer::m_VulkanData;
	VK_CHECK(vkCreateSemaphore(d->s_Device.s_LogicalDevice, &createInfo, d->s_Allocator, outSemaphore));
	return true;
}

bool VulkanSyncObjects::CreateVkFence(VkFence* outFence) {
	VkFenceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	createInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
	VulkanData* d = &VulkanRenderer::m_VulkanData;
	VK_CHECK(vkCreateFence(d->s_Device.s_LogicalDevice, &createInfo, d->s_Allocator, outFence));
	return true;
}

void VulkanSyncObjects::DestroyVkSemaphore(VkSemaphore* semaphore) {
	VulkanData* d = &VulkanRenderer::m_VulkanData;
	vkDestroySemaphore(d->s_Device.s_LogicalDevice, *semaphore, d->s_Allocator);
}

void VulkanSyncObjects::DestroyVkFence(VkFence* fence) {
	VulkanData* d = &VulkanRenderer::m_VulkanData;
	vkDestroyFence(d->s_Device.s_LogicalDevice, *fence, d->s_Allocator);
}