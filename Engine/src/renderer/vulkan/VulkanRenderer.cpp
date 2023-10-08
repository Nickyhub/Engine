#include "VulkanUtils.hpp"
#include "VulkanRenderer.hpp"
#include "VulkanInstance.hpp"
#include "VulkanDevice.hpp"
#include "VulkanSwapchain.hpp"
#include "VulkanRenderpass.hpp"
#include "VulkanFramebuffer.hpp"
#include "VulkanCommandbuffer.hpp"
#include "VulkanBuffer.hpp"

#include "core/Platform.hpp"
#include "core/Application.hpp"

VulkanRenderer::VulkanRenderer(HWND windowHandle, HINSTANCE windowsInstance, unsigned int width, unsigned int height) :
	m_Instance(),
	m_Surface(windowHandle, windowsInstance, m_Instance),
	m_Device(m_Surface, m_Instance, VK_TRUE, VK_TRUE),
	m_Swapchain({width, height, FRAMES_IN_FLIGHT, m_Device, *m_Instance.m_Allocator}),
	m_VulkanImage({ (int) width,
					(int)height,
					VK_FORMAT_R8G8B8A8_SRGB,
					VK_IMAGE_TILING_OPTIMAL,
					VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
					VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
					m_Device,
					*m_Instance.m_Allocator}),

	m_VertexBuffer(VertexBuffer::generatePlaneData(10, 10, 2, 2),
		m_Device,
		*m_Instance.m_Allocator),
	m_Pipeline({ width,
				 height,
				 FRAMES_IN_FLIGHT,
				 m_Swapchain.m_SurfaceFormat.format,
				 m_VertexBuffer,
				 m_Device,
				 *m_Instance.m_Allocator	}),
	m_IndexBuffer(IndexBuffer::generateExampleIndices(), m_Device, *m_Instance.m_Allocator),
	m_UniformBuffer(FRAMES_IN_FLIGHT, m_Device, *m_Instance.m_Allocator) {
	EN_DEBUG("Intializing Vulkan Renderer...");

	m_Swapchain.createFramebuffers(m_Pipeline.m_Renderpass);

	// Create descriptor pool and sets
	m_Pipeline.createDescriptorPool();
	m_Pipeline.createDescriptorSets(m_VulkanImage.m_View, m_UniformBuffer, m_VulkanImage.m_Sampler);

	// Create command buffers
	m_CommandBuffers.resize(FRAMES_IN_FLIGHT);
	for (unsigned int i = 0; i < FRAMES_IN_FLIGHT; i++) {
		m_CommandBuffers[i] = new VulkanCommandbuffer(m_Device, m_Device.m_CommandPool);
	}
}

bool VulkanRenderer::beginFrame() {
	// Wait for the previous frame to finish
	vkWaitForFences(m_Device.m_LogicalDevice, 1, &m_Swapchain.m_InFlightFences[m_Swapchain.m_CurrentFrame]->m_Handle, VK_TRUE, UINT64_MAX);

	if (!m_Swapchain.acquireNextImage(m_Pipeline.m_Renderpass)) {
		EN_DEBUG("Swapchain recreation. Booting.");
		return false;
	}

	vkResetFences(m_Device.m_LogicalDevice, 1, &m_Swapchain.m_InFlightFences[m_Swapchain.m_CurrentFrame]->m_Handle);
	// Reset command buffer
	vkResetCommandBuffer(m_CommandBuffers[m_Swapchain.m_CurrentFrame]->m_Handle, 0);

	if (!m_CommandBuffers[m_Swapchain.m_CurrentFrame]->begin()) {
		EN_ERROR("Failed to record command buffer.");
		return false;
	}

	m_Pipeline.bind(m_CommandBuffers[m_Swapchain.m_CurrentFrame]);
	return true;
}

bool VulkanRenderer::drawFrame() {
	// Maybe this does belong somewhere else
	m_UniformBuffer.update(m_Swapchain.m_Width, m_Swapchain.m_Height, m_Swapchain.m_CurrentFrame);

	m_Pipeline.m_Renderpass.begin(m_Swapchain.m_CurrentSwapchainImageIndex,
								  m_CommandBuffers[m_Swapchain.m_CurrentFrame],
								  m_Swapchain.m_Extent,
								  m_Swapchain.m_Framebuffers);
	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(m_Swapchain.m_Width);
	viewport.height = static_cast<float>(m_Swapchain.m_Height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(m_CommandBuffers[m_Swapchain.m_CurrentFrame]->m_Handle, 0, 1, &viewport);

	// Create scissor
	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = m_Swapchain.m_Extent;
	vkCmdSetScissor(m_CommandBuffers[m_Swapchain.m_CurrentFrame]->m_Handle, 0, 1, &scissor);

	// Bind Buffers
	VkBuffer vertexBuffers[] = {m_VertexBuffer.m_InternalBuffer->m_Handle};
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(m_CommandBuffers[m_Swapchain.m_CurrentFrame]->m_Handle,
							0,
							1,
							vertexBuffers,
							offsets);
	vkCmdBindIndexBuffer(m_CommandBuffers[m_Swapchain.m_CurrentFrame]->m_Handle,
						 m_IndexBuffer.m_InternalBuffer->m_Handle,
						 0,
						 VK_INDEX_TYPE_UINT32);

	vkCmdBindDescriptorSets(m_CommandBuffers[m_Swapchain.m_CurrentFrame]->m_Handle,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		m_Pipeline.m_Layout,
		0,
		1,
		&m_Pipeline.m_DescriptorSets[m_Swapchain.m_CurrentFrame],
		0,
		nullptr);
	vkCmdDrawIndexed(m_CommandBuffers[m_Swapchain.m_CurrentFrame]->m_Handle,
					 static_cast<uint32_t>(m_IndexBuffer.m_Indices->size()),
					 1,
					 0,
					 0,
					 0);
	return true;
}

bool VulkanRenderer::endFrame() {
	if (!m_Pipeline.m_Renderpass.end(m_Swapchain.m_CurrentSwapchainImageIndex,
									m_CommandBuffers[m_Swapchain.m_CurrentFrame])) {
		EN_ERROR("Failed to end renderpass.");
		return false;
	}

	if (!m_CommandBuffers[m_Swapchain.m_CurrentFrame]->end()) {
		EN_ERROR("Failed to end command buffer.");
		return false;
	}

	// Submitting the command buffer
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	VkSemaphore waitSemaphores[] = { m_Swapchain.m_ImageAvailableSemaphores[m_Swapchain.m_CurrentFrame]->m_Handle};
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &m_CommandBuffers[m_Swapchain.m_CurrentFrame]->m_Handle;

	VkSemaphore signalSemaphores[] = { m_Swapchain.m_RenderFinishedSemaphores[m_Swapchain.m_CurrentFrame]->m_Handle};
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	VkResult result = vkQueueSubmit(m_Device.m_GraphicsQueue,
									1,
									&submitInfo,
									m_Swapchain.m_InFlightFences[m_Swapchain.m_CurrentFrame]->m_Handle);
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
		m_Swapchain.recreate({ m_FramebufferWidth,
							   m_FramebufferHeight,
							   FRAMES_IN_FLIGHT,
							   m_Device,
							   *m_Instance.m_Allocator },
							   m_Pipeline.m_Renderpass);
	}
	else if(result != VK_SUCCESS) {
		EN_ERROR("vkQueueSubmit failed with result: %s.", VulkanResultString(result, true));
	}

	m_Swapchain.present();
	m_Swapchain.m_CurrentFrame = (m_Swapchain.m_CurrentFrame + 1) % FRAMES_IN_FLIGHT;
	return true;
}



bool VulkanRenderer::OnResize(const void* sender, EventContext context, EventType type) {
	m_Swapchain.m_Width = context.u32[0];
	m_Swapchain.m_Height = context.u32[1];
	m_FramebufferWidth = context.u32[0];
	m_FramebufferHeight = context.u32[1];
	m_FramebufferGeneration++;
	return true;
}

VulkanRenderer::~VulkanRenderer() {
	// Destroy vulkan objects in the reverse order they were created
	vkDeviceWaitIdle(m_Device.m_LogicalDevice);

	// Destroy command buffers
	for (unsigned int i = 0; i < FRAMES_IN_FLIGHT; i++) {
		delete m_CommandBuffers[i];
	}

	// Destroy debug utils messenger
#ifdef _DEBUG
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_Instance.getInternal(), "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr) {
		func(m_Instance.getInternal(),m_Instance.m_DebugMessenger, m_Instance.m_Allocator);
	}
#endif
	EN_DEBUG("Destroying Vulkan Renderer...");
}