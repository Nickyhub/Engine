#pragma once
#include <vulkan/vulkan.h>

#include "renderer/UniformBufferObject.hpp"
#include "VulkanRenderpass.hpp"
#include "VulkanCommandbuffer.hpp"
#include "VulkanBuffer.hpp"

struct VulkanPipelineConfig {
	unsigned int s_Width;
	unsigned int s_Height;
	unsigned int s_FramesInFlight;

	VkFormat s_RenderpassColorFormat;
	const VertexBuffer& s_VertexBuffer;
	const VulkanDevice& s_Device;
	const VkAllocationCallbacks& s_Allocator;
};

class VulkanPipeline {
public:
	VulkanPipeline() = delete;
	VulkanPipeline(const VulkanPipelineConfig& config);

	void bind(VulkanCommandbuffer* commandbuffer);
	~VulkanPipeline();
	bool createDescriptorPool();
	bool createDescriptorSets(const VkImageView& imageView, const UniformBuffer& uniformBuffer, const VkSampler& sampler);
private:
	bool createDescriptorSetLayout();
private:
	const VulkanDevice& m_Device; // needs to be initialized before the renderpass but still kept private
	const VkAllocationCallbacks& m_Allocator;
public:
	VulkanRenderpass m_Renderpass;
	VkDescriptorSetLayout m_DescriptorSetLayout;
	VkPipelineLayout m_Layout;
	std::vector<VkDescriptorSet> m_DescriptorSets{};
	VkDescriptorPool m_DescriptorPool;
private:
	const unsigned int& m_FramesInFlight;

	VkPipeline m_Handle;
};