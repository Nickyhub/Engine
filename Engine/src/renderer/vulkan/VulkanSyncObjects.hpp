#pragma once

#include <vulkan/vulkan.h>

#include "VulkanDevice.hpp"

class VulkanSemaphore {
public:
	VulkanSemaphore(const VulkanDevice& device, const VkAllocationCallbacks& allocator);
	~VulkanSemaphore();
public:
	VkSemaphore m_Handle;
private:
	const VulkanDevice& m_Device;
	const VkAllocationCallbacks& m_Allocator;
};

class VulkanFence {
public:
	VulkanFence(const VulkanDevice& device, const VkAllocationCallbacks& allocator);
	~VulkanFence();
public:
	VkFence m_Handle;
private:
	const VulkanDevice& m_Device;
	const VkAllocationCallbacks& m_Allocator;
};