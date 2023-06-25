#include "VulkanFramebuffer.hpp"
#include "VulkanRenderer.hpp"

bool VulkanFramebufferUtils::Create(VulkanFramebuffer* outFrambuffer, unsigned int imageIndex) {
	VkImageView attachments[] = {
		VulkanRenderer::m_VulkanData.s_Swapchain.s_ImageViews[imageIndex]
	};

	VkFramebufferCreateInfo framebufferInfo{};
	framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferInfo.renderPass = VulkanRenderer::m_VulkanData.s_Pipeline.s_Renderpass.s_Handle;
	framebufferInfo.attachmentCount = 1;
	framebufferInfo.pAttachments = attachments;
	framebufferInfo.width = VulkanRenderer::m_VulkanData.s_Swapchain.s_Width;
	framebufferInfo.height = VulkanRenderer::m_VulkanData.s_Swapchain.s_Height;
	framebufferInfo.layers = 1;

	VK_CHECK(vkCreateFramebuffer(VulkanRenderer::m_VulkanData.s_Device.s_LogicalDevice, &framebufferInfo, VulkanRenderer::m_VulkanData.s_Allocator, &outFrambuffer->s_Handle));

	return true;
}

void VulkanFramebufferUtils::Destroy(VulkanFramebuffer* frambuffer) {

}
