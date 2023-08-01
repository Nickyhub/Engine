#include "VulkanFramebuffer.hpp"
#include "VulkanRenderer.hpp"
#include "containers/Array.hpp"

bool VulkanFramebufferUtils::Create(VulkanFramebuffer* outFrambuffer, unsigned int imageIndex) {
	Array<VkImageView, 2> attachments;
	attachments[0] = VulkanRenderer::m_VulkanData.s_Swapchain.s_ImageViews[imageIndex];
	attachments[1] = VulkanRenderer::m_VulkanData.s_DepthImage.s_View;

	VkFramebufferCreateInfo framebufferInfo{};
	framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferInfo.renderPass = VulkanRenderer::m_VulkanData.s_Pipeline.s_Renderpass.s_Handle;
	framebufferInfo.attachmentCount = (uint32_t) attachments.Size();
	framebufferInfo.pAttachments = attachments.Data();
	framebufferInfo.width = VulkanRenderer::m_VulkanData.s_Swapchain.s_Width;
	framebufferInfo.height = VulkanRenderer::m_VulkanData.s_Swapchain.s_Height;
	framebufferInfo.layers = 1;

	VK_CHECK(vkCreateFramebuffer(VulkanRenderer::m_VulkanData.s_Device.s_LogicalDevice,
								&framebufferInfo,
								VulkanRenderer::m_VulkanData.s_Allocator,
								&outFrambuffer->s_Handle));

	return true;
}

void VulkanFramebufferUtils::Destroy(VulkanFramebuffer* frambuffer) {

}
