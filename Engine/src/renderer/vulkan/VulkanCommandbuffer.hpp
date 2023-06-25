#pragma once
#include <vulkan/vulkan.h>

enum VulkanCommandbufferState {
	COMMAND_BUFFER_RECORDING,

};

struct VulkanCommandbuffer {
	VkCommandBuffer s_Handle;
	VulkanCommandbufferState s_State;
};

class VulkanCommandbufferUtils {
public:
	static bool Create(VulkanCommandbuffer* outCommandbuffer);
	static bool Record(VulkanCommandbuffer* outCommandbuffer, unsigned int imageIndex);
	static bool End(VulkanCommandbuffer* commandBuffer);
	// No Destroy() because Vulkan frees the buffers automatically when destroying the command pool
};