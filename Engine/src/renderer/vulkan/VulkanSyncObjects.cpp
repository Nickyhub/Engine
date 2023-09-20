#include "VulkanSyncObjects.hpp"
#include "VulkanUtils.hpp"
#include "VulkanDevice.hpp"

#include "core/Logger.hpp"

VulkanSemaphore::VulkanSemaphore(const VulkanDevice& device, const VkAllocationCallbacks& allocator)
	: m_Device(device), m_Allocator(allocator) {
	VkSemaphoreCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	VK_CHECK(vkCreateSemaphore(m_Device.m_LogicalDevice, &createInfo, &m_Allocator, &m_Handle));
}

VulkanSemaphore::~VulkanSemaphore() {
	vkDestroySemaphore(m_Device.m_LogicalDevice, m_Handle, &m_Allocator);
}

VulkanFence::VulkanFence(const VulkanDevice& device, const VkAllocationCallbacks& allocator)
	: m_Device(device), m_Allocator(allocator) {
	VkFenceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	createInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
	VK_CHECK(vkCreateFence(m_Device.m_LogicalDevice, &createInfo, &m_Allocator, &m_Handle));
}

VulkanFence::~VulkanFence() {
	vkDestroyFence(m_Device.m_LogicalDevice, m_Handle, &m_Allocator);
}