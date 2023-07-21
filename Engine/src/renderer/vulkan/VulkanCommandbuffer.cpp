#include "VulkanCommandbuffer.hpp"
#include "VulkanRenderer.hpp"
#include "VulkanUtils.hpp"

bool VulkanCommandbufferUtils::Create(VulkanCommandbuffer* outCommandbuffer) {
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = VulkanRenderer::m_VulkanData.s_Device.s_CommandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = 1;

	VK_CHECK(vkAllocateCommandBuffers(VulkanRenderer::m_VulkanData.s_Device.s_LogicalDevice, &allocInfo, &outCommandbuffer->s_Handle));

	return true;
}

bool VulkanCommandbufferUtils::Record(VulkanCommandbuffer* outCommandbuffer, unsigned int imageIndex) {
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = 0;
	beginInfo.pInheritanceInfo = nullptr;

	VK_CHECK(vkBeginCommandBuffer(outCommandbuffer->s_Handle, &beginInfo));


	return true;
}

VkCommandBuffer VulkanCommandbufferUtils::BeginSingleUseCommands() {
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = VulkanRenderer::m_VulkanData.s_Device.s_CommandPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(VulkanRenderer::m_VulkanData.s_Device.s_LogicalDevice, &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	return commandBuffer;
}

void VulkanCommandbufferUtils::EndSingleUseCommands(VkCommandBuffer commandBuffer) {
	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(VulkanRenderer::m_VulkanData.s_Device.s_GraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(VulkanRenderer::m_VulkanData.s_Device.s_GraphicsQueue);

	vkFreeCommandBuffers(VulkanRenderer::m_VulkanData.s_Device.s_LogicalDevice, VulkanRenderer::m_VulkanData.s_Device.s_CommandPool, 1, &commandBuffer);
}

bool VulkanCommandbufferUtils::End(VulkanCommandbuffer* commandBuffer) {
	//commandBuffer->s_State = 0;
	VK_CHECK(vkEndCommandBuffer(commandBuffer->s_Handle));
	return true;
}
