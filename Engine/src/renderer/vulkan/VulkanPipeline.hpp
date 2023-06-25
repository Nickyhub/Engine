#pragma once
#include <vulkan/vulkan.h>
#include "VulkanRenderpass.hpp"
#include "VulkanCommandbuffer.hpp"

struct VulkanPipeline {
	VkPipeline s_Handle;
	VkPipelineLayout s_Layout;
	VulkanRenderpass s_Renderpass;
};

class VulkanPipelineUtils {
public:
	static bool Create(VulkanPipeline* outPipeline);
	static void Bind(VulkanPipeline* pipeline, VulkanCommandbuffer* commandbuffer);
	static void Destroy(VulkanPipeline* pipeline);
};