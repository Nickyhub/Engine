#pragma once
#include "VulkanPipeline.hpp"
#include "VulkanBuffer.hpp"
#include "VulkanImage.hpp"
#include "VulkanSwapchain.hpp"

#include "core/Event.hpp"

class VulkanRenderer {
public:
	VulkanRenderer() = delete;
	VulkanRenderer(unsigned int width, unsigned int height);
	bool beginFrame();
	bool drawFrame();
	bool endFrame();

	~VulkanRenderer();
	bool OnResize(const void* sender, EventContext context, EventType type);

	VulkanInstance m_Instance;
	VulkanDevice m_Device;
	VulkanSwapchain m_Swapchain;
	VulkanPipeline m_Pipeline;
	DArray<VulkanFramebuffer*> m_Framebuffers;

	// How many frames are simultaneously rendered to (right now: double buffering)
	unsigned int m_FramesInFlight = 2;

	unsigned int m_FramebufferGeneration = 0;
	unsigned int m_LastFramebufferGeneration = 0;

	unsigned int m_FramebufferHeight = 0;
	unsigned int m_FramebufferWidth = 0;

	// This stuff shut be generalized and lie in generic renderbuffers I suppose
	VulkanImage m_VulkanImage;
	VulkanImage m_DepthImage;

	VertexBuffer m_VertexBuffer;
	IndexBuffer m_IndexBuffer;
	UniformBuffer m_UniformBuffer;

	DArray<VulkanCommandbuffer*> m_CommandBuffers{};
};