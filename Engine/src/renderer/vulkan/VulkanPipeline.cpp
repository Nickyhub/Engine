#include "VulkanPipeline.hpp"
#include "VulkanUtils.hpp"
#include "VulkanBuffer.hpp"
#include "VulkanFramebuffer.hpp"

#include "core/File.hpp"
#include "containers/DArray.hpp"
#include "containers/Array.hpp"

VulkanPipeline::VulkanPipeline(const VulkanPipelineConfig& config)
	: m_Device(config.s_Device), m_Allocator(config.s_Allocator),
	m_FramesInFlight(config.s_FramesInFlight),
	m_Renderpass(m_Device, config.s_RenderpassColorFormat, m_Allocator) {
	// First create descriptor set layout
	createDescriptorSetLayout();

	// Read in Shader code
	// TODO maybe shaderconfig file and make this less hardcoded
	File vertexShader, fragmentShader;

	vertexShader.Open("assets/shaders/MaterialShader.vert.spv", FILE_MODE_READ, true);
	fragmentShader.Open("assets/shaders/MaterialShader.frag.spv", FILE_MODE_READ, true);

	DArray<char> vertexShaderSource;
	vertexShaderSource.Resize(vertexShader.Size());
	DArray<char> fragmentShaderSource;
	fragmentShaderSource.Resize(fragmentShader.Size());

	vertexShader.ReadAllBytes(vertexShaderSource.GetData());
	fragmentShader.ReadAllBytes(fragmentShaderSource.GetData());

	vertexShader.Close();
	fragmentShader.Close();

	// Shader modules
	VkShaderModule vertexModule, fragmentModule;

	// Vertex module
	VkShaderModuleCreateInfo vertexCreateInfo{};
	vertexCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	vertexCreateInfo.codeSize = vertexShader.Size();
	vertexCreateInfo.pCode = reinterpret_cast<const uint32_t*>(vertexShaderSource.GetData());

	VK_CHECK(vkCreateShaderModule(m_Device.m_LogicalDevice,
								  &vertexCreateInfo,
								  &m_Allocator,
								  &vertexModule));

	// Fragment module
	VkShaderModuleCreateInfo fragmentCreateInfo{};
	fragmentCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	fragmentCreateInfo.codeSize = fragmentShader.Size();
	fragmentCreateInfo.pCode = reinterpret_cast<const uint32_t*>(fragmentShaderSource.GetData());

	VK_CHECK(vkCreateShaderModule(m_Device.m_LogicalDevice,
								  &fragmentCreateInfo,
								  &m_Allocator,
								  &fragmentModule));

	// Shader stage creation vertex
	VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = vertexModule;
	vertShaderStageInfo.pName = "main";

	// Shader stage creation fragment
	VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = fragmentModule;
	fragShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

	// Dynamic state in order to change viewport size without having to recreate the whole pipeline
	VkDynamicState dynamicStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
	VkPipelineDynamicStateCreateInfo dynamicState{};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = 2;
	dynamicState.pDynamicStates = dynamicStates;

	// Vertex input 
	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	// TODO not hardcode this dude
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.vertexAttributeDescriptionCount = config.s_VertexBuffer.m_AttributeDescriptions.Size();
	vertexInputInfo.pVertexBindingDescriptions = &config.s_VertexBuffer.m_BindingDescription;
	vertexInputInfo.pVertexAttributeDescriptions = &config.s_VertexBuffer.m_AttributeDescriptions[0];

	// Input assembly
	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	// Viewport
	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)config.s_Width;
	viewport.height = (float)config.s_Height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	// Scissor
	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = { config.s_Width, config.s_Height };

	VkPipelineViewportStateCreateInfo viewportStateInfo{};
	viewportStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportStateInfo.viewportCount = 1;
	viewportStateInfo.scissorCount = 1;
	viewportStateInfo.pViewports = &viewport;
	viewportStateInfo.pScissors = &scissor;

	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	// Maybe support wireframe rendering?
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL /*VK_POLYGON_MODE_LINE*/;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f;
	rasterizer.depthBiasClamp = 0.0f;
	rasterizer.depthBiasSlopeFactor = 0.0f;
	rasterizer.lineWidth = 1.0f;

	// Multisampling
	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = 1.0f;
	multisampling.pSampleMask = nullptr;
	multisampling.alphaToCoverageEnable = VK_FALSE;
	multisampling.alphaToOneEnable = VK_FALSE;

	// Color blending
	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

	VkPipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f; // Optional
	colorBlending.blendConstants[1] = 0.0f; // Optional
	colorBlending.blendConstants[2] = 0.0f; // Optional
	colorBlending.blendConstants[3] = 0.0f; // Optional

	// Depth and stencil testing
	VkPipelineDepthStencilStateCreateInfo depthStencil{};
	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencil.depthTestEnable = VK_TRUE;
	depthStencil.depthWriteEnable = VK_TRUE;
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencil.depthBoundsTestEnable = VK_FALSE;
	depthStencil.minDepthBounds = 0.0f; // Optional
	depthStencil.maxDepthBounds = 1.0f; // Optional
	depthStencil.stencilTestEnable = VK_FALSE;
	depthStencil.front = {}; // Optional
	depthStencil.back = {}; // Optional

	// Pipeline layout
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &m_DescriptorSetLayout;
	pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
	pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional
	VK_CHECK(vkCreatePipelineLayout(m_Device.m_LogicalDevice,
									&pipelineLayoutInfo,
									&m_Allocator,
									&m_Layout));

	// Finally creating the pipeline
	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;

	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportStateInfo;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = &depthStencil;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = &dynamicState;

	// Fixed function state
	pipelineInfo.layout = m_Layout;
	pipelineInfo.renderPass = m_Renderpass.m_Handle;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

	VK_CHECK(vkCreateGraphicsPipelines(m_Device.m_LogicalDevice,
										VK_NULL_HANDLE,
										1,
										&pipelineInfo,
										&m_Allocator,
										&m_Handle));
	EN_DEBUG("Graphics pipeline created.");

	// Destroy shader modules as soon as pipeline creation is finishes
	vkDestroyShaderModule(m_Device.m_LogicalDevice,
						  vertexModule,
						  &m_Allocator);
	vkDestroyShaderModule(m_Device.m_LogicalDevice,
						  fragmentModule,
						  &m_Allocator);
}

void VulkanPipeline::bind(VulkanCommandbuffer* commandbuffer) {
	vkCmdBindPipeline(commandbuffer->m_Handle, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Handle);
}

bool VulkanPipeline::createDescriptorPool() {
	DArray<VkDescriptorPoolSize> poolSizes;
	poolSizes.Resize(2);

	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[0].descriptorCount = static_cast<uint32_t>(m_FramesInFlight);
	poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[1].descriptorCount = static_cast<uint32_t>(m_FramesInFlight);

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = 2;
	poolInfo.pPoolSizes = poolSizes.GetData();
	poolInfo.maxSets = static_cast<uint32_t>(m_FramesInFlight);

	VK_CHECK(vkCreateDescriptorPool(m_Device.m_LogicalDevice, &poolInfo, &m_Allocator, &m_DescriptorPool));
	return true;
}

bool VulkanPipeline::createDescriptorSets(const VkImageView& imageView, const UniformBuffer& uniformBuffer, const VkSampler& sampler) {
	// This surely breaks something I guess
	DArray<VkDescriptorSetLayout> layouts;
	for (unsigned int i = 0; i < m_FramesInFlight; i++) {
		layouts.PushBack(m_DescriptorSetLayout);
	}

	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = m_DescriptorPool;
	allocInfo.descriptorSetCount = static_cast<uint32_t>(m_FramesInFlight);
	allocInfo.pSetLayouts = layouts.GetData();

	m_DescriptorSets.Resize(m_FramesInFlight);
	VK_CHECK(vkAllocateDescriptorSets(m_Device.m_LogicalDevice, &allocInfo, m_DescriptorSets.GetData()));

	for (unsigned int i = 0; i < m_FramesInFlight; i++) {
		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = uniformBuffer.m_Buffers[i]->m_Handle;
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(UniformBufferObject);

		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = imageView;
		imageInfo.sampler = sampler;

		DArray<VkWriteDescriptorSet> descriptorWrites;
		descriptorWrites.Resize(2);

		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].dstSet = m_DescriptorSets[i];
		descriptorWrites[0].dstBinding = 0;
		descriptorWrites[0].dstArrayElement = 0;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].pBufferInfo = &bufferInfo;

		descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[1].dstSet = m_DescriptorSets[i];
		descriptorWrites[1].dstBinding = 1;
		descriptorWrites[1].dstArrayElement = 0;
		descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[1].descriptorCount = 1;
		descriptorWrites[1].pImageInfo = &imageInfo;

		vkUpdateDescriptorSets(m_Device.m_LogicalDevice, static_cast<uint32_t>(descriptorWrites.Size()), descriptorWrites.GetData(), 0, nullptr);
	}

	return true;
}

bool VulkanPipeline::createDescriptorSetLayout() {
	VkDescriptorSetLayoutBinding uboLayoutBinding{};
	uboLayoutBinding.binding = 0;
	uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboLayoutBinding.descriptorCount = 1;
	uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	uboLayoutBinding.pImmutableSamplers = nullptr; // Optional

	VkDescriptorSetLayoutBinding samplerLayoutBinding{};
	samplerLayoutBinding.binding = 1;
	samplerLayoutBinding.descriptorCount = 1;
	samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerLayoutBinding.pImmutableSamplers = nullptr;
	samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	Array<VkDescriptorSetLayoutBinding, 2> layoutBindings;
	layoutBindings[0] = uboLayoutBinding;
	layoutBindings[1] = samplerLayoutBinding;

	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = (unsigned int) layoutBindings.Size();
	layoutInfo.pBindings = layoutBindings.Data();

	VK_CHECK(vkCreateDescriptorSetLayout(m_Device.m_LogicalDevice,
										&layoutInfo,
										&m_Allocator,
										&m_DescriptorSetLayout));
	return true;
}

VulkanPipeline::~VulkanPipeline() {
	vkDestroyDescriptorPool(m_Device.m_LogicalDevice, m_DescriptorPool, nullptr);
	vkDestroyDescriptorSetLayout(m_Device.m_LogicalDevice, m_DescriptorSetLayout, &m_Allocator);

	vkDestroyPipeline(m_Device.m_LogicalDevice, m_Handle, &m_Allocator);
	vkDestroyPipelineLayout(m_Device.m_LogicalDevice, m_Layout, &m_Allocator);
}