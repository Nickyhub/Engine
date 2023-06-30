#include "VulkanBuffer.hpp"
#include "containers/Array.hpp"
#include "VulkanRenderer.hpp"
#include "core/Random.hpp"

bool VulkanBufferUtils::CreateBuffer(VkDeviceSize size, VkBufferUsageFlagBits usage, VkMemoryPropertyFlags memoryProperties, VkDeviceMemory &bufferMemory, VkBuffer& buffer) {
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VK_CHECK(vkCreateBuffer(VulkanRenderer::m_VulkanData.s_Device.s_LogicalDevice, &bufferInfo, VulkanRenderer::m_VulkanData.s_Allocator, &buffer));

	// Buffer is created but it needs actual memory associated with it
	VkMemoryRequirements memReq;
	vkGetBufferMemoryRequirements(VulkanRenderer::m_VulkanData.s_Device.s_LogicalDevice, buffer, &memReq);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memReq.size;
	allocInfo.memoryTypeIndex = VulkanBufferUtils::FindMemoryType(memReq.memoryTypeBits, memoryProperties);


	// Allocate and bind the buffer memory
	VK_CHECK(vkAllocateMemory(VulkanRenderer::m_VulkanData.s_Device.s_LogicalDevice, &allocInfo, VulkanRenderer::m_VulkanData.s_Allocator, &bufferMemory));
	vkBindBufferMemory(VulkanRenderer::m_VulkanData.s_Device.s_LogicalDevice, buffer, bufferMemory, 0);

	return true;
}

bool VulkanBufferUtils::CreateVertexBuffer(VertexBuffer& vertexBuffer) {
	if (vertexBuffer.s_Vertices.Size() == 0) {
		EN_WARN("CreateVertexBuffer was called with an empty set of vertices. Nothing happens.");
		return false;
	}

	// First calculate and set the buffer size.
	vertexBuffer.s_Size = vertexBuffer.s_Vertices.Size() * sizeof(vertexBuffer.s_Vertices[0]);


	VkVertexInputBindingDescription bindingDescription{};
	bindingDescription.binding = 0;
	bindingDescription.stride = sizeof(Vertex);
	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	Array<VkVertexInputAttributeDescription, 2> attributeDescriptions;
	attributeDescriptions[0].binding = 0;
	attributeDescriptions[0].location = 0;
	attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[0].offset = offsetof(Vertex, s_Position);

	attributeDescriptions[1].binding = 0;
	attributeDescriptions[1].location = 1;
	attributeDescriptions[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	attributeDescriptions[1].offset = offsetof(Vertex, s_Colour);

	// Copy the produced data to the buffer
	vertexBuffer.s_AttributeDescriptions.PushBack(attributeDescriptions[1]);
	vertexBuffer.s_AttributeDescriptions.PushBack(attributeDescriptions[0]);

	vertexBuffer.s_BindingDescription = bindingDescription;

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	if (!CreateBuffer(
		vertexBuffer.s_Size,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		stagingBufferMemory,
		stagingBuffer)) {
		EN_ERROR("Failed to create staging buffer while calling CreateVertexBuffer.");
		return false;
	}

	// Map the memory of the geometry data to the staging buffers memory
	void* data;
	vkMapMemory(VulkanRenderer::m_VulkanData.s_Device.s_LogicalDevice, stagingBufferMemory, 0, vertexBuffer.s_Size, 0, &data);
	memcpy(data, vertexBuffer.s_Vertices.GetData(), (size_t)vertexBuffer.s_Size);
	vkUnmapMemory(VulkanRenderer::m_VulkanData.s_Device.s_LogicalDevice, stagingBufferMemory);
	
	if (!CreateBuffer(
		vertexBuffer.s_Size,
		(VkBufferUsageFlagBits)(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT),
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		vertexBuffer.s_Memory, vertexBuffer.s_Handle)) {
		EN_ERROR("Failed to create device local vertex buffer.");
		return false;
	}

	CopyBuffer(stagingBuffer, vertexBuffer.s_Handle, vertexBuffer.s_Size);

	// Clean up staging buffer
	vkDestroyBuffer(VulkanRenderer::m_VulkanData.s_Device.s_LogicalDevice, stagingBuffer, VulkanRenderer::m_VulkanData.s_Allocator);
	vkFreeMemory(VulkanRenderer::m_VulkanData.s_Device.s_LogicalDevice, stagingBufferMemory, VulkanRenderer::m_VulkanData.s_Allocator);
	return true;
}

bool VulkanBufferUtils::CreateIndexBuffer(IndexBuffer& indexBuffer) {
	if (indexBuffer.s_Indices.Size() == 0) {
		EN_WARN("CreateIndexBuffer was called with an empty set of vertices. Nothing happens.");
		return false;
	}
	// Calculate and set index buffer size
	indexBuffer.s_Size = sizeof(unsigned int) * indexBuffer.s_Indices.Size();

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	if (!CreateBuffer(indexBuffer.s_Size,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		stagingBufferMemory, 
		stagingBuffer)) {
		EN_ERROR("Failed to create index staging buffer.");
		return false;
	}

	void* data;
	vkMapMemory(VulkanRenderer::m_VulkanData.s_Device.s_LogicalDevice, stagingBufferMemory, 0, indexBuffer.s_Size, 0, &data);
	memcpy(data, indexBuffer.s_Indices.GetData(), (size_t)indexBuffer.s_Size);
	vkUnmapMemory(VulkanRenderer::m_VulkanData.s_Device.s_LogicalDevice, stagingBufferMemory);

	if (!CreateBuffer(indexBuffer.s_Size,
		(VkBufferUsageFlagBits)(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT),
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		indexBuffer.s_Memory,
		indexBuffer.s_Handle)) {
		EN_ERROR("Failed to create device local index buffer.");
		return false;
	}

	CopyBuffer(stagingBuffer, indexBuffer.s_Handle, indexBuffer.s_Size);

	// Clean up staging buffer
	vkDestroyBuffer(VulkanRenderer::m_VulkanData.s_Device.s_LogicalDevice, stagingBuffer, VulkanRenderer::m_VulkanData.s_Allocator);
	vkFreeMemory(VulkanRenderer::m_VulkanData.s_Device.s_LogicalDevice, stagingBufferMemory, VulkanRenderer::m_VulkanData.s_Allocator);
	return true;
}

bool VulkanBufferUtils::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
	// Get ptr to vulkan data for cosmetic reasons
	VulkanData* vulkanData = &VulkanRenderer::m_VulkanData;

	// Create command Buffer to copy original buffer
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = vulkanData->s_Device.s_CommandPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	VK_CHECK(vkAllocateCommandBuffers(vulkanData->s_Device.s_LogicalDevice, &allocInfo, &commandBuffer));

	// Begin to record command buffer
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	VK_CHECK(vkBeginCommandBuffer(commandBuffer, &beginInfo));

	VkBufferCopy copyRegion{};
	copyRegion.srcOffset = 0;
	copyRegion.dstOffset = 0;
	copyRegion.size = size;
	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
	VK_CHECK(vkEndCommandBuffer(commandBuffer));


	// Execute recorded command buffer
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	VK_CHECK(vkQueueSubmit(vulkanData->s_Device.s_GraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE));
	VK_CHECK(vkQueueWaitIdle(vulkanData->s_Device.s_GraphicsQueue));

	// Clean up command buffer
	vkFreeCommandBuffers(vulkanData->s_Device.s_LogicalDevice, vulkanData->s_Device.s_CommandPool, 1, &commandBuffer);
	return true;
}

unsigned int VulkanBufferUtils::FindMemoryType(unsigned int typeFilter, VkMemoryPropertyFlags properties) {
	// Find the proper memory type
	VkPhysicalDeviceMemoryProperties memProps;
	vkGetPhysicalDeviceMemoryProperties(VulkanRenderer::m_VulkanData.s_Device.s_PhysicalDevice, &memProps);
	for (unsigned int i = 0; i < memProps.memoryTypeCount; i++) {
		if ((typeFilter & (1 << i)) && (memProps.memoryTypes[i].propertyFlags & (properties))) {
			return i;
		}
	}
	return 0;
}

void VulkanBufferUtils::Destroy(VulkanBuffer* buffer) {
	vkDestroyBuffer(VulkanRenderer::m_VulkanData.s_Device.s_LogicalDevice, buffer->s_Handle, VulkanRenderer::m_VulkanData.s_Allocator);
	vkFreeMemory(VulkanRenderer::m_VulkanData.s_Device.s_LogicalDevice, buffer->s_Memory, VulkanRenderer::m_VulkanData.s_Allocator);
}

bool VulkanBufferUtils::GeneratePlaneData(VertexBuffer* outVertexBuffer, IndexBuffer* outIndexBuffer, unsigned int width, unsigned int height, unsigned int fieldWidth, unsigned int fieldHeight) {

	/*for (unsigned int i = 0; i < height; i++) {
		for (unsigned int j = 0; j < width; j++) {
			glm::vec3 v = { i* fieldWidth, j* fieldHeight, Random::GetRandomNumberInWholeRange(-1, 1) };
			glm::vec4 c = { Random::GetNormalizedFloat(), Random::GetNormalizedFloat() + 4, Random::GetNormalizedFloat(), 1.0f};
			outBuffer->s_Vertices.PushBack({ v,c });
		}
	}*/

	outVertexBuffer->s_Vertices.PushBack({ {-0.5f, -0.5f, 0.5f}, { 1.0f, 0.0f, 0.0f, 0.0f } });
	outVertexBuffer->s_Vertices.PushBack({ {0.5f, -0.5f, 0.5f}, {0.0f, 1.0f, 0.0f, 0.0f} });
	outVertexBuffer->s_Vertices.PushBack({ {0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 1.0f, 0.0f} });
	outVertexBuffer->s_Vertices.PushBack({ {-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f, 0.0f} });

	outIndexBuffer->s_Indices.PushBack(0);
	outIndexBuffer->s_Indices.PushBack(1);
	outIndexBuffer->s_Indices.PushBack(2);
	outIndexBuffer->s_Indices.PushBack(2);
	outIndexBuffer->s_Indices.PushBack(3);
	outIndexBuffer->s_Indices.PushBack(0);

	return true;
}