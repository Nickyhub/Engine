#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <chrono>

#include "VulkanBuffer.hpp"
#include "VulkanCommandbuffer.hpp"
#include "VulkanUtils.hpp"
#include "containers/Array.hpp"
#include "core/Random.hpp"

VulkanBuffer::VulkanBuffer(const VulkanDevice& device,
	VkDeviceSize size,
	VkBufferUsageFlagBits usage,
	VkMemoryPropertyFlags memPropertyFlags,
	const VkAllocationCallbacks& allocator)
	: m_Device(device), m_Allocator(allocator), m_Size(size) {
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VK_CHECK(vkCreateBuffer(m_Device.m_LogicalDevice,
		&bufferInfo,
		&m_Allocator,
		&m_Handle));

	// Buffer is created but it needs actual memory associated with it
	VkMemoryRequirements memReq;
	vkGetBufferMemoryRequirements(m_Device.m_LogicalDevice, m_Handle, &memReq);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memReq.size;
	allocInfo.memoryTypeIndex = findMemoryType(m_Device, memReq.memoryTypeBits, memPropertyFlags);

	// Allocate and bind the buffer memory
	VK_CHECK(vkAllocateMemory(m_Device.m_LogicalDevice,
		&allocInfo,
		&m_Allocator,
		&m_Memory));
	vkBindBufferMemory(m_Device.m_LogicalDevice,
		m_Handle,
		m_Memory,
		0);
}

bool VulkanBuffer::copyBuffer(VkBuffer dstBuffer, VkDeviceSize size, VkCommandPool pool, VkQueue queue) {
	VulkanCommandbuffer commandBuffer(m_Device, pool);
	commandBuffer.beginSingleUseCommands();

	VkBufferCopy copyRegion{};
	copyRegion.size = size;
	vkCmdCopyBuffer(commandBuffer.m_Handle, m_Handle, dstBuffer, 1, &copyRegion);

	commandBuffer.endSingleUseCommands(queue);
	return true;
}

int VulkanBuffer::findMemoryType(const VulkanDevice& device, unsigned int typeFilter, VkMemoryPropertyFlags properties) {
	// Find the proper memory type
	VkPhysicalDeviceMemoryProperties memProps;
	vkGetPhysicalDeviceMemoryProperties(device.m_PhysicalDevice, &memProps);
	for (unsigned int i = 0; i < memProps.memoryTypeCount; i++) {
		if ((typeFilter & (1 << i)) && (memProps.memoryTypes[i].propertyFlags & (properties))) {
			return i;
		}
	}
	return 0;
}

VulkanBuffer::~VulkanBuffer() {
	vkDestroyBuffer(m_Device.m_LogicalDevice,
		m_Handle,
		&m_Allocator);
	vkFreeMemory(m_Device.m_LogicalDevice,
		m_Memory,
		&m_Allocator);
}

VertexBuffer::VertexBuffer(DArray<Vertex>* vertices,
	const VulkanDevice& device,
	const VkAllocationCallbacks& allocator)
	: m_Device(device), m_Allocator(allocator), m_Vertices(vertices) {
	if (vertices->Size() == 0) {
		EN_WARN("CreateVertexBuffer was called with an empty set of vertices. Nothing happens.");
	}

	// First calculate and set the buffer size.
	unsigned int bufferSize = m_Vertices->Size() * sizeof(m_Vertices[0]);
	VkVertexInputBindingDescription bindingDescription{};
	bindingDescription.binding = 0;
	bindingDescription.stride = sizeof(Vertex);
	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	Array<VkVertexInputAttributeDescription, 3> attributeDescriptions;
	// inPosition
	attributeDescriptions[0].binding = 0;
	attributeDescriptions[0].location = 0;
	attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[0].offset = offsetof(Vertex, s_Position);

	// inColor
	attributeDescriptions[1].binding = 0;
	attributeDescriptions[1].location = 1;
	attributeDescriptions[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	attributeDescriptions[1].offset = offsetof(Vertex, s_Colour);

	// inTexCoord
	attributeDescriptions[2].binding = 0;
	attributeDescriptions[2].location = 2;
	attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
	attributeDescriptions[2].offset = offsetof(Vertex, s_TexCoord);

	// Copy the produced data to the buffer
	m_AttributeDescriptions.PushBack(attributeDescriptions[0]);
	m_AttributeDescriptions.PushBack(attributeDescriptions[1]);
	m_AttributeDescriptions.PushBack(attributeDescriptions[2]);

	m_BindingDescription = bindingDescription;

	VulkanBuffer stagingBuffer(m_Device,
		bufferSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		m_Allocator);

	// Map the memory of the geometry data to the staging buffers memory
	void* data;
	vkMapMemory(m_Device.m_LogicalDevice,
		stagingBuffer.m_Memory,
		0,
		bufferSize,
		0,
		&data);
	memcpy(data, m_Vertices->GetData(), (size_t)bufferSize);
	vkUnmapMemory(m_Device.m_LogicalDevice, stagingBuffer.m_Memory);

	m_InternalBuffer = new VulkanBuffer(m_Device,
		bufferSize,
		(VkBufferUsageFlagBits)(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT),
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		m_Allocator);


	stagingBuffer.copyBuffer(m_InternalBuffer->m_Handle,
							 bufferSize,
							 m_Device.m_CommandPool,
							 m_Device.m_GraphicsQueue);
	// Staging buffer will be automatically destroyed when destructor is called at the
	// end of this method
}

DArray<Vertex>* VertexBuffer::generatePlaneData(unsigned int width, unsigned int height, unsigned int fieldWidth, unsigned int fieldHeight) {
	/*for (unsigned int i = 0; i < height; i++) {
		for (unsigned int j = 0; j < width; j++) {
			glm::vec3 v = { i* fieldWidth, j* fieldHeight, Random::GetRandomNumberInWholeRange(-1, 1) };
			glm::vec4 c = { Random::GetNormalizedFloat(), Random::GetNormalizedFloat() + 4, Random::GetNormalizedFloat(), 1.0f};
			outBuffer->s_Vertices.PushBack({ v,c });
		}
	}*/
	DArray<Vertex>* vertices = new DArray<Vertex>();
	vertices->PushBack({ {-0.5f, -0.5f, 0.0f}, { 1.0f, 0.0f, 0.0f, 0.0f}, {1.0f, 0.0f} });
	vertices->PushBack({ {0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f, 0.0f}, {0.0f, 0.0f} });
	vertices->PushBack({ {0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f, 0.0f}, {0.0f, 1.0f} });
	vertices->PushBack({ {-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f, 0.0f}, {1.0f, 1.0f} });

	vertices->PushBack({ {-0.5f, -0.5f, 1.0f}, { 1.0f, 0.0f, 0.0f, 0.0f}, {1.0f, 0.0f} });
	vertices->PushBack({ {0.5f, -0.5f, 1.0f}, {0.0f, 1.0f, 0.0f, 0.0f}, {0.0f, 0.0f} });
	vertices->PushBack({ {0.5f, 0.5f, 1.0f}, {0.0f, 0.0f, 1.0f, 0.0f}, {0.0f, 1.0f} });
	vertices->PushBack({ {-0.5f, 0.5f, 1.0f}, {1.0f, 1.0f, 1.0f, 0.0f}, {1.0f, 1.0f} });
	return vertices;
}

VertexBuffer::~VertexBuffer() {
	delete m_InternalBuffer;
	delete m_Vertices;
}


IndexBuffer::IndexBuffer(DArray<unsigned int>* indices,
	const VulkanDevice& device,
	const VkAllocationCallbacks& allocator)
	: m_Indices(indices), m_Device(device), m_Allocator(allocator) {
	if (m_Indices->Size() == 0) {
		EN_WARN("CreateIndexBuffer was called with an empty set of vertices. Nothing happens.");
	}
	// Calculate and set index buffer size
	unsigned int bufferSize = sizeof(unsigned int) * m_Indices->Size();

	VulkanBuffer stagingBuffer(m_Device,
		bufferSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		m_Allocator);

	void* data;
	vkMapMemory(m_Device.m_LogicalDevice,
		stagingBuffer.m_Memory,
		0,
		bufferSize,
		0,
		&data);
	memcpy(data, m_Indices->GetData(), (size_t)bufferSize);
	vkUnmapMemory(m_Device.m_LogicalDevice,
		stagingBuffer.m_Memory);

	m_InternalBuffer = new VulkanBuffer(m_Device,
		bufferSize,
		(VkBufferUsageFlagBits)(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT),
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		m_Allocator);

	stagingBuffer.copyBuffer(m_InternalBuffer->m_Handle,
							 bufferSize,
							 m_Device.m_CommandPool,
							 m_Device.m_GraphicsQueue);
	// Clean up staging buffer(destructor will be called since its stack allocated)
}

DArray<unsigned int>* IndexBuffer::generateExampleIndices() {
	DArray<unsigned int>* indices = new DArray<unsigned int>();
	indices->PushBack(0);
	indices->PushBack(1);
	indices->PushBack(2);
	indices->PushBack(2);
	indices->PushBack(3);
	indices->PushBack(0);

	indices->PushBack(4);
	indices->PushBack(5);
	indices->PushBack(6);
	indices->PushBack(6);
	indices->PushBack(7);
	indices->PushBack(4);

	return indices;
}

IndexBuffer::~IndexBuffer() {
	delete m_InternalBuffer;
	delete m_Indices;
}

UniformBuffer::UniformBuffer(unsigned int framesInFlight, const VulkanDevice& device, const VkAllocationCallbacks& allocator)
	: m_Device(device), m_Allocator(allocator) {
	VkDeviceSize bufferSize = sizeof(UniformBufferObject);

	m_Buffers.Resize(framesInFlight);
	m_UniformBuffersMapped.Resize(framesInFlight);

	for (unsigned int i = 0; i < framesInFlight; i++) {
		m_Buffers[i] = new VulkanBuffer(device,
			bufferSize,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			m_Allocator);
	}
}

UniformBuffer::~UniformBuffer() {
	for (unsigned int i = 0; i < m_Buffers.Size(); i++) {
		delete m_Buffers[i];
	}
}

void UniformBuffer::update(unsigned int width, unsigned int height, unsigned int currentFrame) {
	static auto startTime = std::chrono::high_resolution_clock::now();

	auto currentTime = std::chrono::high_resolution_clock::now();
	float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

	UniformBufferObject ubo{};
	ubo.s_Model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	ubo.s_View = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	ubo.s_Proj = glm::perspective(
		glm::radians(45.0f),
		(float)width / height,
		0.1f,
		10.0f);
	ubo.s_Proj[1][1] *= -1;

	Memory::Copy(m_UniformBuffersMapped[currentFrame], &ubo, sizeof(ubo));
}

