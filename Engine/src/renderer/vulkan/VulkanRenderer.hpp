#pragma once
#include "VulkanUtils.hpp"
#include "core/Event.hpp"

class VulkanRenderer {
public:
	static bool Initialize();
	static bool BeginFrame(VulkanData* data);
	static bool DrawFrame(VulkanData* data);
	static bool EndFrame(VulkanData* data);

	static void Shutdown();
	static bool OnResize(const void* sender, EventContext context, EventType type);

	static VulkanData m_VulkanData;
};