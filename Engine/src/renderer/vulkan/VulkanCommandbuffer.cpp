#include "VulkanCommandbuffer.hpp"
#include "VulkanUtils.hpp"

VulkanCommandbuffer::VulkanCommandbuffer(const VulkanDevice& device, const VkCommandPool& pool) 
	: m_Device(device), m_Pool(pool) {
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = pool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = 1;

	VK_CHECK(vkAllocateCommandBuffers(m_Device.m_LogicalDevice,
									  &allocInfo,
									  &m_Handle));
}

bool VulkanCommandbuffer::record() {
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = 0;
	beginInfo.pInheritanceInfo = nullptr;

	VK_CHECK(vkBeginCommandBuffer(m_Handle, &beginInfo));
	return true;
}

VkCommandBuffer VulkanCommandbuffer::beginSingleUseCommands() {
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = m_Pool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(m_Device.m_LogicalDevice, &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	return commandBuffer;
}

void VulkanCommandbuffer::endSingleUseCommands(VkQueue queue) {
	vkEndCommandBuffer(m_Handle);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &m_Handle;

	vkQueueSubmit(queue,
				  1,
				  &submitInfo,
				  VK_NULL_HANDLE);
	vkQueueWaitIdle(queue);

	vkFreeCommandBuffers(m_Device.m_LogicalDevice,
						 m_Pool,
						 1,
						 &m_Handle);
}

bool VulkanCommandbuffer::end() {
	//commandBuffer->s_State = 0;
	VK_CHECK(vkEndCommandBuffer(m_Handle));
	return true;
}
