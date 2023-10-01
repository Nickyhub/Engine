#pragma once
#include <vulkan/vulkan.h>
#include "VulkanDevice.hpp"

enum VulkanCommandbufferState {
	COMMAND_BUFFER_RECORDING,
};

class VulkanCommandbuffer {
public:
	VulkanCommandbuffer() = delete;
	VulkanCommandbuffer(const VulkanDevice& device, const VkCommandPool& pool);
	bool begin();
	bool end();

	static VkCommandBuffer beginSingleUseCommands(const VulkanDevice& device, const VkCommandPool& pool);
	static void endSingleUseCommands(const VkCommandBuffer& commandBuffer,
									 const VkQueue& queue,
									 const VulkanDevice& device,
									 const VkCommandPool& pool);
	// No Destroy() because Vulkan frees the buffers automatically when destroying the command pool
public:
	VkCommandBuffer m_Handle;
private:
	const VulkanDevice& m_Device;
	const VkCommandPool& m_Pool;
};