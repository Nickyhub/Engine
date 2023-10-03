#include "VulkanFramebuffer.hpp"
#include "VulkanUtils.hpp"
#include "VulkanDevice.hpp"

#include "core/Logger.hpp"
#include "containers/Array.hpp"

VulkanFramebuffer::VulkanFramebuffer(const VulkanFramebufferConfig& config) 
	: m_Device(config.s_Device), m_Allocator(config.s_Allocator) {

	Array<VkImageView, 2> attachments;
	attachments[0] = config.s_Swapchain.m_ImageViews[config.imageIndex];
	attachments[1] = config.s_DepthImage.m_View;

	VkFramebufferCreateInfo framebufferInfo{};
	framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferInfo.renderPass = config.s_Renderpass.m_Handle;
	framebufferInfo.attachmentCount = (uint32_t) attachments.Size();
	framebufferInfo.pAttachments = attachments.Data();
	framebufferInfo.width = config.s_Width;
	framebufferInfo.height = config.s_Height;
	framebufferInfo.layers = 1;

	VK_CHECK(vkCreateFramebuffer(m_Device.m_LogicalDevice,
								&framebufferInfo,
								&m_Allocator,
								&m_Handle));
}

VulkanFramebuffer::~VulkanFramebuffer() {
	vkDestroyFramebuffer(m_Device.m_LogicalDevice, m_Handle, &m_Allocator);
	EN_DEBUG("Vulkan framebuffer destroyed.");
}
