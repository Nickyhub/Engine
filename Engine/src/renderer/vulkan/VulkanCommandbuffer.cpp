#include "VulkanCommandbuffer.hpp"
#include "VulkanUtils.hpp"

#include "core/Logger.hpp"

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

bool VulkanCommandbuffer::begin() {
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = 0;
	beginInfo.pInheritanceInfo = nullptr;

	VK_CHECK(vkBeginCommandBuffer(m_Handle, &beginInfo));
	return true;
}

VkCommandBuffer VulkanCommandbuffer::beginSingleUseCommands(const VulkanDevice& device, const VkCommandPool& pool) {
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = pool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(device.m_LogicalDevice, &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	VK_CHECK(vkBeginCommandBuffer(commandBuffer, &beginInfo));
	return commandBuffer;
}

void VulkanCommandbuffer::endSingleUseCommands(const VkCommandBuffer& commandBuffer,
											   const VkQueue& queue,
											   const VulkanDevice& device, 
											   const VkCommandPool& pool) {
	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(queue,
				  1,
				  &submitInfo,
				  VK_NULL_HANDLE);
	vkQueueWaitIdle(queue);

	vkFreeCommandBuffers(device.m_LogicalDevice,
						 pool,
						 1,
						 &commandBuffer);
}

bool VulkanCommandbuffer::end() {
	//commandBuffer->s_State = 0;
	VK_CHECK(vkEndCommandBuffer(m_Handle));
	return true;
}
