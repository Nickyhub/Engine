#pragma once

#include <vulkan/vulkan.h>
#include "containers/DArray.hpp"
#include "glm/glm.hpp"

#include "renderer/UniformBufferObject.hpp"

struct Vertex {
	glm::vec3 s_Position;
	glm::vec4 s_Colour;
	glm::vec2 s_TexCoord;
};

struct VulkanBuffer {
	VkDeviceSize s_Size = 0;
	VkBuffer s_Handle{};
	VkDeviceMemory s_Memory{};
};

struct VertexBuffer : VulkanBuffer {
	DArray<Vertex> s_Vertices;
	DArray<VkVertexInputAttributeDescription> s_AttributeDescriptions;
	VkVertexInputBindingDescription s_BindingDescription{ };
};

struct IndexBuffer : VulkanBuffer {
	DArray<unsigned int> s_Indices;
};

struct UniformBuffer : VulkanBuffer {
	DArray<VkBuffer> s_UniformBuffers;
	DArray<VkDeviceMemory> s_UniformBuffersMemory;
	DArray<void*> s_UniformBuffersMapped;
	UniformBufferObject s_BufferObject{};
};

class VulkanBufferUtils {
public:
	static bool CreateBuffer(VkDeviceSize size, VkBufferUsageFlagBits usage, VkMemoryPropertyFlags flags, VkDeviceMemory& bufferMemory, VkBuffer& buffer);
	static bool CreateVertexBuffer(VertexBuffer& buffer);
	static bool CreateIndexBuffer(IndexBuffer& buffer);
	static bool CreateUniformBuffer(UniformBuffer& buffer);
	static void UpdateUniformBuffer(UniformBuffer& buffer);
	static bool CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	static void Destroy(VulkanBuffer* buffer);
	static bool GeneratePlaneData(VertexBuffer* outVertexBuffer, IndexBuffer* outIndexBuffer, unsigned int width, unsigned int height, unsigned int fieldWidth, unsigned int fieldHeight);
	static unsigned int FindMemoryType(unsigned int typeFilter, VkMemoryPropertyFlags properties);
};