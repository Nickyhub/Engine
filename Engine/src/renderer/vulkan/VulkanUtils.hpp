#pragma once

#include <vulkan/vulkan.h>
#include "core/Logger.hpp"
#include "VulkanInstance.hpp"
#include "VulkanDevice.hpp"
#include "VulkanSwapchain.hpp"
#include "VulkanPipeline.hpp"
#include "VulkanRenderpass.hpp"
#include "VulkanFramebuffer.hpp"
#include "VulkanBuffer.hpp"
#include "VulkanImage.hpp"

#define VK_CHECK(expression)								\
	if(expression != VK_SUCCESS) {							\
		EN_ERROR("Error executing %s", ##expression);		\
		return false;										\
	}

struct VulkanData {
	VkSurfaceKHR s_Surface = nullptr;
	VkInstance s_Instance = nullptr;
	VkAllocationCallbacks* s_Allocator = nullptr;
	VulkanDevice s_Device{};
	VulkanSwapchain s_Swapchain{};
	VulkanPipeline s_Pipeline{};
	
	// How many frames are simultaneously rendered to (right now: double buffering)
	unsigned int s_FramesInFlight = 2;
	unsigned int s_CurrentFrame = 0;
	
	unsigned int s_FramebufferGeneration = 0;
	unsigned int s_LastFramebufferGeneration = 0;

	unsigned int s_FramebufferHeight = 0;
	unsigned int s_FramebufferWidth = 0;

	// This stuff shut be generalized and lie in generic renderbuffers I suppose
	VulkanImage s_VulkanImage{};
	VertexBuffer s_VertexBuffer{};
	IndexBuffer s_IndexBuffer{};
	UniformBuffer s_UniformBuffer{};

	VkDescriptorPool s_DescriptorPool{};
	DArray<VkDescriptorSet> s_DescriptorSets{};

	DArray<VulkanCommandbuffer> s_CommandBuffers{};

	// Sync Objects
	DArray<VkSemaphore> s_ImageAvailableSemaphores{};
	DArray<VkSemaphore> s_RenderFinishedSemaphores{};
	DArray<VkFence> s_InFlightFences{};
#ifdef _DEBUG
	VkDebugUtilsMessengerEXT s_DebugMessenger = nullptr;
#endif
};

const char* VulkanResultString(VkResult result, bool getExtended);