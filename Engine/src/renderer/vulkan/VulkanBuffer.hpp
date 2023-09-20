#pragma once

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

#include "containers/DArray.hpp"
#include "renderer/UniformBufferObject.hpp"
#include "VulkanDevice.hpp"

struct Vertex {
	glm::vec3 s_Position;
	glm::vec4 s_Colour;
	glm::vec2 s_TexCoord;
};

class VulkanBuffer {
public:
	VulkanBuffer() = delete;
	VulkanBuffer(const VulkanDevice& device,
				 VkDeviceSize size,
				 VkBufferUsageFlagBits usage,
				 VkMemoryPropertyFlags flags,
				 const VkAllocationCallbacks& allocator);
	bool copyBuffer(VkBuffer dstBuffer, VkDeviceSize size, VkCommandPool pool, VkQueue queue);
	~VulkanBuffer();
	static int findMemoryType(const VulkanDevice& device, unsigned int typeFilter, VkMemoryPropertyFlags properties);
public:
	VkBuffer m_Handle{};
	VkDeviceMemory m_Memory{};
private:
	const VulkanDevice& m_Device;
	const VkAllocationCallbacks& m_Allocator;
	VkDeviceSize m_Size = 0;
};

class VertexBuffer {
public:
	VertexBuffer() = delete;
	VertexBuffer(DArray<Vertex>* vertices,
				 const VulkanDevice& device,
				 const VkAllocationCallbacks& allocator);
	static DArray<Vertex>* generatePlaneData(unsigned int width,
						   unsigned int height,
						   unsigned int fieldWidth,
						   unsigned int fieldHeight);
	~VertexBuffer();
public:
	VulkanBuffer* m_InternalBuffer;
	DArray<VkVertexInputAttributeDescription> m_AttributeDescriptions;
	VkVertexInputBindingDescription m_BindingDescription{ };
private:
	const VulkanDevice& m_Device;
	const VkAllocationCallbacks& m_Allocator;
	DArray<Vertex>* m_Vertices;
};

class IndexBuffer {
public:
	IndexBuffer(DArray<unsigned int>* indices,
				const VulkanDevice& device,
				const VkAllocationCallbacks& allocator);
	~IndexBuffer();
	static DArray<unsigned int>* generateExampleIndices();
public:
	VulkanBuffer* m_InternalBuffer;
	DArray<unsigned int>* m_Indices;
private:
	const VulkanDevice& m_Device;
	const VkAllocationCallbacks& m_Allocator;
};

class UniformBuffer {
public:
	UniformBuffer(unsigned int framesInFlight, const VulkanDevice& device, const VkAllocationCallbacks& allocator);
	~UniformBuffer();
	void update(unsigned int width, unsigned int height, unsigned int currentFrame);
public:
	DArray<VulkanBuffer*> m_Buffers;
private:
	const VulkanDevice& m_Device;
	const VkAllocationCallbacks& m_Allocator;
	DArray<void*> m_UniformBuffersMapped;
	UniformBufferObject s_BufferObject{};
};
