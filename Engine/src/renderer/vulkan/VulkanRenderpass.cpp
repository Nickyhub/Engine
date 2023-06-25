#include "VulkanRenderpass.hpp"
#include "VulkanRenderer.hpp"
#include "VulkanCommandbuffer.hpp"

bool VulkanRenderpassUtils::Create(VulkanRenderpass* outRenderpass) {
	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = VulkanRenderer::m_VulkanData.s_Swapchain.s_SurfaceFormat.format;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;

	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colorAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	VulkanData* d = &VulkanRenderer::m_VulkanData;
	VK_CHECK(vkCreateRenderPass(d->s_Device.s_LogicalDevice, &renderPassInfo, d->s_Allocator, &d->s_Pipeline.s_Renderpass.s_Handle));

	EN_DEBUG("Renderpass created.");
	return true;
}

bool VulkanRenderpassUtils::Begin(VulkanRenderpass* renderpass, unsigned int imageIndex, VulkanCommandbuffer* commandBuffer) {
	VulkanSwapchain* swapchain = &VulkanRenderer::m_VulkanData.s_Swapchain;
	if (imageIndex <= swapchain->s_Framebuffers.Capacity()) {
		VkRenderPassBeginInfo renderpassInfo{};
		renderpassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderpassInfo.renderPass = renderpass->s_Handle;
		renderpassInfo.framebuffer = swapchain->s_Framebuffers[imageIndex].s_Handle;
		renderpassInfo.renderArea.offset = { 0, 0 };
		renderpassInfo.renderArea.extent = VulkanRenderer::m_VulkanData.s_Swapchain.s_Extent;

		VkClearValue clearColor{ {{ 0.0f, 0.1f, 0.1f}} };
		renderpassInfo.clearValueCount = 1;
		renderpassInfo.pClearValues = &clearColor;

		vkCmdBeginRenderPass(commandBuffer->s_Handle, &renderpassInfo, VK_SUBPASS_CONTENTS_INLINE);
		return true;
	}
	else {
		EN_ERROR("Invalid image index passed to VulkanRenderpassCreate::Begin: %u.", imageIndex);
		return false;
	}
}

bool VulkanRenderpassUtils::End(VulkanRenderpass* renderpass, unsigned int imageIndex, VulkanCommandbuffer* commandBuffer) {
	vkCmdEndRenderPass(commandBuffer->s_Handle);
	return true;
}

void VulkanRenderpassUtils::Destroy(VulkanRenderpass* renderpass) {
	vkDestroyRenderPass(VulkanRenderer::m_VulkanData.s_Device.s_LogicalDevice, renderpass->s_Handle, VulkanRenderer::m_VulkanData.s_Allocator);
}