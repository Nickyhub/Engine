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
	bool record();
	bool end();

	VkCommandBuffer beginSingleUseCommands();
	void endSingleUseCommands(VkQueue queue);
	// No Destroy() because Vulkan frees the buffers automatically when destroying the command pool
public:
	VkCommandBuffer m_Handle;
private:
	const VulkanDevice& m_Device;
	const VkCommandPool& m_Pool;
};