#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/gtc/matrix_transform.hpp>
#include <chrono>

#include "VulkanBuffer.hpp"
#include "VulkanCommandbuffer.hpp"
#include "VulkanUtils.hpp"

#include "containers/Array.hpp"
#include "core/Random.hpp"
#include "core/Memory.hpp"
#include "core/Logger.hpp"

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

bool VulkanBuffer::copyBuffer(VkBuffer dstBuffer, VkDeviceSize size, VkQueue queue) {
	VkCommandBuffer commandBuffer = VulkanCommandbuffer::beginSingleUseCommands(m_Device, m_Device.m_CommandPool);

	VkBufferCopy copyRegion{};
	copyRegion.size = size;
	vkCmdCopyBuffer(commandBuffer, m_Handle, dstBuffer, 1, &copyRegion);

	VulkanCommandbuffer::endSingleUseCommands(commandBuffer, queue, m_Device, m_Device.m_CommandPool);
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
	EN_WARN("No appropriate memory type has been found. Returning 0.");
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

VertexBuffer::VertexBuffer(std::vector<Vertex>* vertices,
	const VulkanDevice& device,
	const VkAllocationCallbacks& allocator)
	: m_Device(device), m_Allocator(allocator), m_Vertices(vertices) {
	if (vertices->size() == 0) {
		EN_WARN("CreateVertexBuffer was called with an empty set of vertices. Nothing happens.");
	}


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
	attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[1].offset = offsetof(Vertex, s_Colour);

	// inTexCoord
	attributeDescriptions[2].binding = 0;
	attributeDescriptions[2].location = 2;
	attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
	attributeDescriptions[2].offset = offsetof(Vertex, s_TexCoord);

	// Copy the produced data to the buffer
	m_AttributeDescriptions.push_back(attributeDescriptions[0]);
	m_AttributeDescriptions.push_back(attributeDescriptions[1]);
	m_AttributeDescriptions.push_back(attributeDescriptions[2]);

	m_BindingDescription = bindingDescription;

	// First calculate and set the buffer size.
	size_t bufferSize = m_Vertices->size() * sizeof(m_Vertices[0]);

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
	memcpy(data, m_Vertices->data(), m_Vertices->size() * sizeof(m_Vertices[0]));
	vkUnmapMemory(m_Device.m_LogicalDevice, stagingBuffer.m_Memory);

	m_InternalBuffer = new VulkanBuffer(m_Device,
		bufferSize,
		(VkBufferUsageFlagBits)(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT),
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		m_Allocator);


	if (!stagingBuffer.copyBuffer(m_InternalBuffer->m_Handle,
		bufferSize,
		m_Device.m_GraphicsQueue)) {
		EN_ERROR("Failed to copy staging buffer to actual vulkan buffer.");
	}
	// Staging buffer will be automatically destroyed when destructor is called at the
	// end of this method
}

std::vector<Vertex>* VertexBuffer::generatePlaneData(unsigned int width, unsigned int height, unsigned int fieldWidth, unsigned int fieldHeight) {
	/*for (unsigned int i = 0; i < height; i++) {
		for (unsigned int j = 0; j < width; j++) {
			glm::vec3 v = { i* fieldWidth, j* fieldHeight, Random::GetRandomNumberInWholeRange(-1, 1) };
			glm::vec4 c = { Random::GetNormalizedFloat(), Random::GetNormalizedFloat() + 4, Random::GetNormalizedFloat(), 1.0f};
			outBuffer->s_Vertices.PushBack({ v,c });
		}
	}*/
	std::vector<Vertex>* vertices = new std::vector<Vertex>();
	vertices->push_back({ {-0.5f, -0.5f, 0.0f}, { 1.0f, 0.0f, 0.0f}, {1.0f, 0.0f} });
	vertices->push_back({ {0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f} });
	vertices->push_back({ {0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f} });
	vertices->push_back({ {-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f} });

	vertices->push_back({ {-0.5f, -0.5f, 1.0f}, { 1.0f, 0.0f, 0.0f}, {1.0f, 0.0f} });
	vertices->push_back({ {0.5f, -0.5f, 1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f} });
	vertices->push_back({ {0.5f, 0.5f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f} });
	vertices->push_back({ {-0.5f, 0.5f, 1.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f} });
	/*for (int i = 0; i < 8; i++) {
		vertices->push_back({ {0 + float(i * 8), 1 + float(i * 8), 2 + float(i * 8)}, { 3 + float(i * 8), 4 + float(i * 8), 5 + float(i * 8)}, {6 + float(i * 8), 7 + float(i * 8)} });
	}*/
	return vertices;
}

VertexBuffer::~VertexBuffer() {
	delete m_InternalBuffer;
	delete m_Vertices;
	EN_DEBUG("Vertex buffer destroyed.");
}


IndexBuffer::IndexBuffer(std::vector<unsigned int>* indices,
	const VulkanDevice& device,
	const VkAllocationCallbacks& allocator)
	: m_Indices(indices), m_Device(device), m_Allocator(allocator) {
	if (m_Indices->size() == 0) {
		EN_WARN("CreateIndexBuffer was called with an empty set of vertices. Nothing happens.");
	}
	// Calculate and set index buffer size
	size_t bufferSize = sizeof(unsigned int) * m_Indices->size();

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
	memcpy(data, m_Indices->data(), bufferSize);
	vkUnmapMemory(m_Device.m_LogicalDevice,
		stagingBuffer.m_Memory);

	m_InternalBuffer = new VulkanBuffer(m_Device,
		bufferSize,
		(VkBufferUsageFlagBits)(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT),
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		m_Allocator);

	stagingBuffer.copyBuffer(m_InternalBuffer->m_Handle,
							 bufferSize,
							 m_Device.m_GraphicsQueue);
	// Clean up staging buffer(destructor will be called since its stack allocated)
	EN_INFO("Index buffer created.");
}

std::vector<unsigned int>* IndexBuffer::generateExampleIndices() {
	std::vector<unsigned int>* indices = new std::vector<unsigned int>();
	indices->push_back(0);
	indices->push_back(1);
	indices->push_back(2);
	indices->push_back(2);
	indices->push_back(3);
	indices->push_back(0);

	indices->push_back(4);
	indices->push_back(5);
	indices->push_back(6);
	indices->push_back(6);
	indices->push_back(7);
	indices->push_back(4);

	return indices;
}

IndexBuffer::~IndexBuffer() {
	delete m_InternalBuffer;
	delete m_Indices;
	EN_DEBUG("Index buffer destroyed.");
}

UniformBuffer::UniformBuffer(unsigned int framesInFlight, const VulkanDevice& device, const VkAllocationCallbacks& allocator)
	: m_Device(device), m_Allocator(allocator) {
	VkDeviceSize bufferSize = sizeof(UniformBufferObject);

	m_Buffers.resize(framesInFlight);
	m_UniformBuffersMapped.resize(framesInFlight);

	for (unsigned int i = 0; i < framesInFlight; i++) {
		m_Buffers[i] = new VulkanBuffer(device,
			bufferSize,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			m_Allocator);
		vkMapMemory(m_Device.m_LogicalDevice,
			m_Buffers[i]->m_Memory,
			0,
			bufferSize,
			0,
			&m_UniformBuffersMapped[i]);
	}
	EN_INFO("Uniform buffer created.");
}

UniformBuffer::~UniformBuffer() {
	for (unsigned int i = 0; i < m_Buffers.size(); i++) {
		delete m_Buffers[i];
	}
	EN_DEBUG("Uniform buffer destroyed.");
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