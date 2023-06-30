#include "VulkanPipeline.hpp"
#include "VulkanUtils.hpp"
#include "VulkanRenderer.hpp"

#include "core/File.hpp"
#include "containers/DArray.hpp"

bool VulkanPipelineUtils::Create(VulkanPipeline* outPipeline) {
	// Read in Shader code
	// TODO maybe shaderconfig file and make this less hardcoded
	File vertexShader, fragmentShader;

	vertexShader.Open("assets/shaders/MaterialShader.vert.spv", FILE_MODE_READ, true);
	fragmentShader.Open("assets/shaders/MaterialShader.frag.spv", FILE_MODE_READ, true);

	DArray<char> vertexShaderSource(vertexShader.Size());
	DArray<char> fragmentShaderSource(fragmentShader.Size());

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

	VK_CHECK(vkCreateShaderModule(VulkanRenderer::m_VulkanData.s_Device.s_LogicalDevice, &vertexCreateInfo, VulkanRenderer::m_VulkanData.s_Allocator, &vertexModule));

	// Fragment module
	VkShaderModuleCreateInfo fragmentCreateInfo{};
	fragmentCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	fragmentCreateInfo.codeSize = fragmentShader.Size();
	fragmentCreateInfo.pCode = reinterpret_cast<const uint32_t*>(fragmentShaderSource.GetData());

	VK_CHECK(vkCreateShaderModule(VulkanRenderer::m_VulkanData.s_Device.s_LogicalDevice, &fragmentCreateInfo, VulkanRenderer::m_VulkanData.s_Allocator, &fragmentModule));

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
	vertexInputInfo.vertexAttributeDescriptionCount = 2;
	vertexInputInfo.pVertexBindingDescriptions = &VulkanRenderer::m_VulkanData.s_VertexBuffer.s_BindingDescription;
	vertexInputInfo.pVertexAttributeDescriptions = &VulkanRenderer::m_VulkanData.s_VertexBuffer.s_AttributeDescriptions[0];

	// Input assembly
	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	// Viewport
	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)VulkanRenderer::m_VulkanData.s_Swapchain.s_Width;
	viewport.height = (float)VulkanRenderer::m_VulkanData.s_Swapchain.s_Height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	// Scissor
	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = VulkanRenderer::m_VulkanData.s_Swapchain.s_Extent;

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
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
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

	// Depth and stencil testing
	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

	// Color blending
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

	// Pipeline layout
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 0; // Optional
	pipelineLayoutInfo.pSetLayouts = nullptr; // Optional
	pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
	pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional
	VK_CHECK(vkCreatePipelineLayout(VulkanRenderer::m_VulkanData.s_Device.s_LogicalDevice, &pipelineLayoutInfo, VulkanRenderer::m_VulkanData.s_Allocator, &outPipeline->s_Layout));

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
	pipelineInfo.pDepthStencilState = nullptr;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = &dynamicState;

	// Fixed function state
	pipelineInfo.layout = outPipeline->s_Layout;
	pipelineInfo.renderPass = outPipeline->s_Renderpass.s_Handle;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

	VK_CHECK(vkCreateGraphicsPipelines(VulkanRenderer::m_VulkanData.s_Device.s_LogicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, VulkanRenderer::m_VulkanData.s_Allocator, &outPipeline->s_Handle));
	EN_DEBUG("Graphics pipeline created.");

	// Destroy shader modules as soon as pipeline creation is finishes
	vkDestroyShaderModule(VulkanRenderer::m_VulkanData.s_Device.s_LogicalDevice, vertexModule, VulkanRenderer::m_VulkanData.s_Allocator);
	vkDestroyShaderModule(VulkanRenderer::m_VulkanData.s_Device.s_LogicalDevice, fragmentModule, VulkanRenderer::m_VulkanData.s_Allocator);

	return true;
}

void VulkanPipelineUtils::Bind(VulkanPipeline* pipeline, VulkanCommandbuffer* commandbuffer) {
	vkCmdBindPipeline(commandbuffer->s_Handle, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->s_Handle);
}

void VulkanPipelineUtils::Destroy(VulkanPipeline* pipeline) {
	vkDestroyPipeline(VulkanRenderer::m_VulkanData.s_Device.s_LogicalDevice, pipeline->s_Handle, VulkanRenderer::m_VulkanData.s_Allocator);
	vkDestroyPipelineLayout(VulkanRenderer::m_VulkanData.s_Device.s_LogicalDevice, pipeline->s_Layout, VulkanRenderer::m_VulkanData.s_Allocator);
}

