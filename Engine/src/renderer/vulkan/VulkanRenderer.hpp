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

private:
	VulkanInstance m_Instance;
	VulkanDevice m_Device;
	VulkanSwapchain m_Swapchain;
	std::vector<VulkanFramebuffer*> m_Framebuffers;

	// How many frames are simultaneously rendered to (right now: double buffering)
	unsigned int m_FramesInFlight = 2;

	unsigned int m_FramebufferGeneration = 0;
	unsigned int m_LastFramebufferGeneration = 0;

	unsigned int m_FramebufferHeight = 0;
	unsigned int m_FramebufferWidth = 0;

	// Textured vulkan image for texturing demonstration purposes
	VulkanImage m_VulkanImage;
	VulkanImage m_DepthImage;

	VertexBuffer m_VertexBuffer;
	VulkanPipeline m_Pipeline;

	IndexBuffer m_IndexBuffer;
	UniformBuffer m_UniformBuffer;

	std::vector<VulkanCommandbuffer*> m_CommandBuffers{};
};