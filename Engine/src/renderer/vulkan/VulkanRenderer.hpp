#pragma once

#include <windows.h>

#include "VulkanPipeline.hpp"
#include "VulkanBuffer.hpp"
#include "VulkanImage.hpp"
#include "VulkanSwapchain.hpp"

#include "core/Event.hpp"

	// How many frames are simultaneously rendered to (right now: double buffering)
#define FRAMES_IN_FLIGHT 2

class VulkanRenderer {
public:
	VulkanRenderer() = delete;
	VulkanRenderer(HWND windowHandle, HINSTANCE windowsInstance, unsigned int width, unsigned int height);
	bool beginFrame();
	bool drawFrame();
	bool endFrame();

	~VulkanRenderer();
	bool OnResize(const void* sender, EventContext context, EventType type);

private:
	VulkanInstance m_Instance;
	VulkanSurface m_Surface;
	VulkanDevice m_Device;
	VulkanSwapchain m_Swapchain;

	unsigned int m_FramebufferGeneration = 0;
	unsigned int m_LastFramebufferGeneration = 0;

	unsigned int m_FramebufferHeight = 0;
	unsigned int m_FramebufferWidth = 0;

	// Textured vulkan image for texturing demonstration purposes
	VulkanImage m_VulkanImage;

	VertexBuffer m_VertexBuffer;
	VulkanPipeline m_Pipeline;

	IndexBuffer m_IndexBuffer;
	UniformBuffer m_UniformBuffer;

	std::vector<VulkanCommandbuffer*> m_CommandBuffers{};
};