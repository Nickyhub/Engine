#include "VulkanRenderer.hpp"
#include "VulkanInstance.hpp"
#include "VulkanDevice.hpp"
#include "VulkanSwapchain.hpp"
#include "VulkanRenderpass.hpp"
#include "VulkanFramebuffer.hpp"
#include "VulkanCommandbuffer.hpp"
#include "VulkanSyncObjects.hpp"
#include "VulkanBuffer.hpp"

#include "core/Platform.hpp"
#include "core/Application.hpp"

VulkanData VulkanRenderer::m_VulkanData;

bool VulkanRenderer::Initialize() {
	EN_DEBUG("Intializing Vulkan Renderer...");

	// Create vulkan objects in certain order
	if (!VulkanInstance::Create(&m_VulkanData.s_Instance)) {
		EN_ERROR("Renderer failed to create Vulkan Instance. Shutting down.");
		return false;
	}

	// Create surface
	if (!Platform::CreateVulkanSurface(m_VulkanData)) {
		EN_ERROR("Failed to create vulkan surface. Shutting down.");
		return false;
	}

	// Create device (logical and physical)
	if (!VulkanDeviceUtils::Create(&m_VulkanData.s_Device)) {
		EN_ERROR("Failed to create vulkan device. Shutting down.");
		return false;
	}

	// Create swapchain 
	if (!VulkanSwapchainUtils::Create(&m_VulkanData.s_Swapchain, Application::GetConfig().Width, Application::GetConfig().Height)) {
		EN_ERROR("Failed to create vulkan swapchain. Shutting down.");
		return false;
	}

	// Create the Renderpass
	if (!VulkanRenderpassUtils::Create(&m_VulkanData.s_Pipeline.s_Renderpass)) {
		EN_ERROR("Failed to create vulkan renderpass. Shutting down.");
		return false;
	}

	// Create Framebuffers
	//Pointer for convenience
	m_VulkanData.s_Swapchain.s_Framebuffers.Resize(VulkanRenderer::m_VulkanData.s_Swapchain.s_ImageCount);
	for (unsigned int i = 0; i < VulkanRenderer::m_VulkanData.s_Swapchain.s_ImageCount; i++) {
		if (!VulkanFramebufferUtils::Create(&m_VulkanData.s_Swapchain.s_Framebuffers[i], i)) {
			EN_ERROR("Failed to create framebuffer for image index %d.", i);
			return false;
		}
	}

	// Create VertexBuffer Right before the graphics pipeline
	if (!VulkanBufferUtils::GeneratePlaneData(&m_VulkanData.s_VertexBuffer, &m_VulkanData.s_IndexBuffer, 10, 10, 2, 2) ||
		!VulkanBufferUtils::CreateVertexBuffer(m_VulkanData.s_VertexBuffer)
		//!VulkanBufferUtils::CreateIndexBuffer(m_VulkanData.s_IndexBuffer
		) {
		EN_ERROR("Failed to create vertex buffer. Shutting down.");
		return false;
	}

	// Create graphics pipeline
	if (!VulkanPipelineUtils::Create(&m_VulkanData.s_Pipeline)) {
		EN_ERROR("Failed to create vulkan pipeline. Shutting down.");
		return false;
	}

	m_VulkanData.s_CommandBuffers.Resize(m_VulkanData.s_FramesInFlight);

	// Create Command buffers
	for (unsigned int i = 0; i < m_VulkanData.s_FramesInFlight; i++) {
		if (!VulkanCommandbufferUtils::Create(&m_VulkanData.s_CommandBuffers[i])) {
			EN_ERROR("Failed to create vulkan command buffer. Shutting down.");
			return false;
		}
	}

	// Create sync objects
	m_VulkanData.s_ImageAvailableSemaphores.Resize(m_VulkanData.s_FramesInFlight);
	m_VulkanData.s_RenderFinishedSemaphores.Resize(m_VulkanData.s_FramesInFlight);
	m_VulkanData.s_InFlightFences.Resize(m_VulkanData.s_FramesInFlight);
	for (unsigned int i = 0; i < m_VulkanData.s_FramesInFlight; i++) {
		if (!VulkanSyncObjects::CreateVkSemaphore(&m_VulkanData.s_ImageAvailableSemaphores[i]) ||
			!VulkanSyncObjects::CreateVkSemaphore(&m_VulkanData.s_RenderFinishedSemaphores[i]) ||
			!VulkanSyncObjects::CreateVkFence(&m_VulkanData.s_InFlightFences[i])) {
			EN_ERROR("Failed to create vulkan sync objects. Shutting down.");
			return false;
		}
	}


	return true;
}

bool VulkanRenderer::BeginFrame(VulkanData* data) {
	// Wait for the previous frame to finish
	vkWaitForFences(data->s_Device.s_LogicalDevice, 1, &data->s_InFlightFences[data->s_CurrentFrame], VK_TRUE, UINT64_MAX);

	if (!VulkanSwapchainUtils::AcquireNextImage(&data->s_Swapchain)) {
		EN_DEBUG("Swapchain recreation. Booting.");
		return false;
	}

	vkResetFences(data->s_Device.s_LogicalDevice, 1, &data->s_InFlightFences[data->s_CurrentFrame]);
	// Reset command buffer
	vkResetCommandBuffer(data->s_CommandBuffers[data->s_CurrentFrame].s_Handle, 0);

	VulkanCommandbufferUtils::Record(&data->s_CommandBuffers[data->s_CurrentFrame], data->s_Swapchain.s_CurrentSwapchainImageIndex);
	VulkanPipelineUtils::Bind(&data->s_Pipeline, &data->s_CommandBuffers[data->s_CurrentFrame]);
	return true;
}

bool VulkanRenderer::DrawFrame(VulkanData* data) {
	VulkanRenderpassUtils::Begin(&data->s_Pipeline.s_Renderpass, data->s_Swapchain.s_CurrentSwapchainImageIndex, &data->s_CommandBuffers[data->s_CurrentFrame]);

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(data->s_Swapchain.s_Width);
	viewport.height = static_cast<float>(data->s_Swapchain.s_Height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(data->s_CommandBuffers[data->s_CurrentFrame].s_Handle, 0, 1, &viewport);

	// Create scissor
	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = data->s_Swapchain.s_Extent;
	vkCmdSetScissor(data->s_CommandBuffers[data->s_CurrentFrame].s_Handle, 0, 1, &scissor);

	// Bind Buffers
	VkBuffer vertexBuffers[] = {m_VulkanData.s_VertexBuffer.s_Handle};
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(data->s_CommandBuffers[data->s_CurrentFrame].s_Handle, 0, 1, vertexBuffers, offsets);
	//vkCmdBindIndexBuffer(data->s_CommandBuffers[data->s_CurrentFrame].s_Handle, data->s_IndexBuffer.s_Handle, 0, VK_INDEX_TYPE_UINT32);

	//vkCmdDrawIndexed(data->s_CommandBuffers[data->s_CurrentFrame].s_Handle, static_cast<uint32_t>(data->s_IndexBuffer.s_Indices.Size()), 1, 0, 0, 0);
	vkCmdDraw(data->s_CommandBuffers[data->s_CurrentFrame].s_Handle, static_cast<uint32_t>(m_VulkanData.s_VertexBuffer.s_Vertices.Size()), 1, 0, 0);
	return true;
}

bool VulkanRenderer::EndFrame(VulkanData* data) {
	if (!VulkanRenderpassUtils::End(&data->s_Pipeline.s_Renderpass, data->s_Swapchain.s_CurrentSwapchainImageIndex, &data->s_CommandBuffers[data->s_CurrentFrame])) {
		EN_ERROR("Failed to end renderpass.");
		return false;
	}

	if (!VulkanCommandbufferUtils::End(&data->s_CommandBuffers[data->s_CurrentFrame])) {
		EN_ERROR("Failed to end command buffer.");
		return false;
	}

	// Submitting the command buffer
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	VkSemaphore waitSemaphores[] = { data->s_ImageAvailableSemaphores[data->s_CurrentFrame]};
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &data->s_CommandBuffers[data->s_CurrentFrame].s_Handle;

	VkSemaphore signalSemaphores[] = { data->s_RenderFinishedSemaphores[data->s_CurrentFrame]};
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	VkResult result = vkQueueSubmit(data->s_Device.s_GraphicsQueue, 1, &submitInfo, data->s_InFlightFences[data->s_CurrentFrame]);
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
		VulkanSwapchainUtils::Recreate(&data->s_Swapchain, data->s_FramebufferWidth, data->s_FramebufferHeight);
	}
	else if(result != VK_SUCCESS) {
		EN_ERROR("vkQueueSubmit failed with result: %s.", VulkanResultString(result, true));
	}

	VulkanSwapchainUtils::Present(&data->s_Swapchain);
	m_VulkanData.s_CurrentFrame = (m_VulkanData.s_CurrentFrame + 1) % m_VulkanData.s_FramesInFlight;
	return true;
}



bool VulkanRenderer::OnResize(const void* sender, EventContext context, EventType type) {
	m_VulkanData.s_FramebufferWidth = context.u32[0];
	m_VulkanData.s_FramebufferHeight = context.u32[1];
	m_VulkanData.s_FramebufferGeneration++;
	return true;
}

void VulkanRenderer::Shutdown() {
	// Destroy vulkan objects in the reverse order they were created
	vkDeviceWaitIdle(m_VulkanData.s_Device.s_LogicalDevice);

	VulkanBufferUtils::Destroy(&m_VulkanData.s_IndexBuffer);
	VulkanBufferUtils::Destroy(&m_VulkanData.s_VertexBuffer);

	// Destroy sync objects
	for (unsigned int i = 0; i < m_VulkanData.s_FramesInFlight; i++) {
		VulkanSyncObjects::DestroyVkSemaphore(&m_VulkanData.s_RenderFinishedSemaphores[i]);
		VulkanSyncObjects::DestroyVkSemaphore(&m_VulkanData.s_ImageAvailableSemaphores[i]);
		VulkanSyncObjects::DestroyVkFence(&m_VulkanData.s_InFlightFences[i]);
	}

	VulkanPipelineUtils::Destroy(&m_VulkanData.s_Pipeline);
	VulkanRenderpassUtils::Destroy(&m_VulkanData.s_Pipeline.s_Renderpass);
	VulkanSwapchainUtils::Destroy(&m_VulkanData.s_Swapchain);
	VulkanDeviceUtils::Destroy(&m_VulkanData.s_Device);

	// Destroy debug utils messenger
#ifdef _DEBUG
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_VulkanData.s_Instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr) {
		func(m_VulkanData.s_Instance, m_VulkanData.s_DebugMessenger, m_VulkanData.s_Allocator);
	}
#endif

	Platform::DestroyVulkanSurface(m_VulkanData);
	VulkanInstance::Destroy(&m_VulkanData.s_Instance);
}