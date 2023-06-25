#pragma once

#include <vulkan/vulkan.h>
#include "containers/DArray.hpp"
#include "glm/glm.hpp"

struct Vertex {
	glm::vec3 s_Position;
	glm::vec4 s_Colour;
};

struct VulkanBuffer {
	VkDeviceSize s_Size = 0;
	VkBuffer s_Handle{};
	VkDeviceMemory s_Memory{};
};

struct VertexBuffer : VulkanBuffer {
	DArray<Vertex> s_Vertices;
	DArray<VkVertexInputAttributeDescription> s_AttributeDescriptions;
	VkVertexInputBindingDescription s_BindingDescription{ 0 };
};

struct IndexBuffer : VulkanBuffer {
	DArray<unsigned int> s_Indices;
};

class VulkanBufferUtils {
public:
	static bool CreateBuffer(VkDeviceSize size, VkBufferUsageFlagBits usage, VkMemoryPropertyFlags flags, VkDeviceMemory& bufferMemory, VkBuffer& buffer);
	static bool CreateVertexBuffer(VertexBuffer& buffer);
	static bool CreateIndexBuffer(IndexBuffer& buffer);
	static bool CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	static void Destroy(VulkanBuffer* buffer);
	static bool GeneratePlaneData(VertexBuffer* outVertexBuffer, IndexBuffer* outIndexBuffer, unsigned int width, unsigned int height, unsigned int fieldWidth, unsigned int fieldHeight);
private:
	static unsigned int FindMemoryType(unsigned int typeFilter, VkMemoryPropertyFlags properties);
};