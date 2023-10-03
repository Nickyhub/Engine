#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#endif

#include <stb_image.h>

#include "VulkanImage.hpp"
#include "VulkanBuffer.hpp"
#include "VulkanCommandbuffer.hpp"
#include "VulkanUtils.hpp"

#include "core/Application.hpp"
#include "core/Memory.hpp"

/**
 * Depending on the usage a depth image and its view will be created. If it is a colored image it
 * will be loaded and created with a staging buffer.
 * TODO Do not hard code the path of colored image. Wrap this in a general texture system
 */
VulkanImage::VulkanImage(const VulkanImageConfig& config)
	: m_Device(config.s_Device),
	  m_Allocator(config.s_Allocator) {
	switch (config.s_Usage) {
		case VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT: {
			createImage(config);
			createImageView(config.s_Format, VK_IMAGE_ASPECT_DEPTH_BIT);
			break;
		}
		case VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT: {
			stbi_uc* pixels = stbi_load("assets/textures/texture.jpg", &m_Width, &m_Height, &m_Channels, STBI_rgb_alpha);
			VkDeviceSize imageSize = (VkDeviceSize)(m_Width * m_Height * 4);

			if (!pixels) {
				EN_ERROR("Failed to load image from disk.");
			}

			// Create staging buffer
			VulkanBuffer stagingBuffer(m_Device,
				imageSize,
				VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
				(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT),
				m_Allocator);

			// Copy pixel data to staging buffer
			void* data;
			vkMapMemory(m_Device.m_LogicalDevice, stagingBuffer.m_Memory, 0, imageSize, 0, &data);
			Memory::Copy(data, pixels, (unsigned int)imageSize);
			vkUnmapMemory(m_Device.m_LogicalDevice, stagingBuffer.m_Memory);

			// Clean up pixel data
			stbi_image_free(pixels);

			// TODO: Should use config width and height. When a texture system is in place it will load
			// the texture from disk and provide the width and height for the vulkan image from outside.
			// Outside meaning a generic texture class that uses VulkanImage class in the background.
			if (!createImage({m_Width,
							  m_Height,
							  config.s_Format,
							  config.s_Tiling,
							  config.s_Usage,
							  config.s_Properties, 
							  config.s_Device, 
							  config.s_Allocator})) {
				EN_ERROR("Failed to create vulkan image.");
			}

			// Transition the image layout and copy the staging buffers content to the vulkan image
			transitionImageLayout(m_Handle,
				VK_FORMAT_R8G8B8A8_SRGB,
				VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
			copyBufferToImage(stagingBuffer.m_Handle, m_Width, m_Height);

			// Transition the layout again for the shader
			transitionImageLayout(m_Handle,
				VK_FORMAT_R8G8B8A8_SRGB,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

			if (!createImageView(VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT)) {
				EN_ERROR("Failed to create Image view.");
			}

			if (!createTextureSampler(m_Sampler)) {
				EN_ERROR("Failed to create texture image sampler.");
			}
			// Cleanup (destructor of VulkanBuffer will be called here for staging buffer)
			break;
		}
	};
}

bool VulkanImage::createImage(const VulkanImageConfig& config) {
	// Create the vulkan image
	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = config.s_Width;
	imageInfo.extent.height = config.s_Height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.format = config.s_Format;
	imageInfo.tiling = config.s_Tiling;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = config.s_Usage;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.flags = 0; // Optional

	VK_CHECK(vkCreateImage(m_Device.m_LogicalDevice, &imageInfo, &m_Allocator, &m_Handle));
	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(m_Device.m_LogicalDevice, m_Handle, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = VulkanBuffer::findMemoryType(m_Device, memRequirements.memoryTypeBits, config.s_Properties);

	if (vkAllocateMemory(m_Device.m_LogicalDevice, &allocInfo, nullptr, &m_Memory) != VK_SUCCESS) {
		EN_ERROR("Failed to allocate image memory.");
		return false;
	}

	vkBindImageMemory(m_Device.m_LogicalDevice, m_Handle, m_Memory, 0);
	return true;
}

bool VulkanImage::createImageView(VkFormat format, VkImageAspectFlags aspectFlags) {
	VkImageViewCreateInfo viewInfo{};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = m_Handle;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = format;
	viewInfo.subresourceRange.aspectMask = aspectFlags;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;

	VK_CHECK(vkCreateImageView(m_Device.m_LogicalDevice, &viewInfo, &m_Allocator, &m_View));
	return true;
}

bool VulkanImage::createTextureSampler(VkSampler& outSampler) {
	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;

	// Figure out max anisotropy
	VkPhysicalDeviceProperties properties{};
	vkGetPhysicalDeviceProperties(m_Device.m_PhysicalDevice, &properties);
	samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = 0.0f;

	VK_CHECK(vkCreateSampler(m_Device.m_LogicalDevice, &samplerInfo, &m_Allocator, &outSampler));
	return true;
}

bool VulkanImage::createDepthImage(const VulkanImageConfig& config) {
	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(m_Device.m_LogicalDevice, m_Handle, &memRequirements);
	// Create VkImage
	if (!createImage(config)) {
		EN_ERROR("Failed to create VkImage for depth image.");
		return false;
	}
	return true;
}

VkFormat VulkanImage::findSupportedFormat(std::vector<VkFormat> candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {
	for (unsigned int i = 0; i < candidates.size(); i++) {
		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(m_Device.m_PhysicalDevice, candidates[i], &props);
		if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
			return candidates[i];
		}
		else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
			return candidates[i];
		}
	}
	EN_WARN("Failed to find supported image format for depth image. NULL is returned.");
	return {};
}

void VulkanImage::transitionImageLayout(VkImage image,
	VkFormat format,
	VkImageLayout oldLayout,
	VkImageLayout newLayout) {
	VkCommandBuffer commandBuffer = VulkanCommandbuffer::beginSingleUseCommands(m_Device, m_Device.m_CommandPool);

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	VkPipelineStageFlags sourceStage{};
	VkPipelineStageFlags destinationStage{};

	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	}
	else {
		EN_ERROR("Unsupported image layout transition.");
	}

	if (newLayout != VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	}

	vkCmdPipelineBarrier(
		commandBuffer,
		sourceStage, destinationStage,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier
	);
	VulkanCommandbuffer::endSingleUseCommands(commandBuffer, m_Device.m_GraphicsQueue, m_Device, m_Device.m_CommandPool);
}

void VulkanImage::copyBufferToImage(const VkBuffer& buffer,
	uint32_t width,
	uint32_t height) {
	VkCommandBuffer commandBuffer = VulkanCommandbuffer::beginSingleUseCommands(m_Device, m_Device.m_CommandPool);

	VkBufferImageCopy region{};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;

	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;

	region.imageOffset = { 0, 0, 0 };
	region.imageExtent = {
		width,
		height,
		1
	};

	vkCmdCopyBufferToImage(
		commandBuffer,
		buffer,
		m_Handle,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1,
		&region
	);
	VulkanCommandbuffer::endSingleUseCommands(commandBuffer, m_Device.m_GraphicsQueue, m_Device, m_Device.m_CommandPool);
}

VulkanImage::~VulkanImage() {
	if (m_Sampler != NULL) {
		vkDestroySampler(m_Device.m_LogicalDevice, m_Sampler, &m_Allocator);
	}
	vkDestroyImageView(m_Device.m_LogicalDevice, m_View, &m_Allocator);
	vkDestroyImage(m_Device.m_LogicalDevice, m_Handle, &m_Allocator);
	vkFreeMemory(m_Device.m_LogicalDevice, m_Memory, &m_Allocator);

	EN_DEBUG("Vulkan image destroyed.");
}