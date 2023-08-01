#ifndef STB_IMAGE_IMPLEMENTATION
	#define STB_IMAGE_IMPLEMENTATION
#endif

#include <stb_image.h>

#include "VulkanImage.hpp"
#include "VulkanBuffer.hpp"
#include "VulkanRenderer.hpp"
#include "core/Logger.hpp"

/**
 * Creates a VkImage with staging buffer. Transitions the Layout and automatically creates VkImageView as well as the texture sampler
 * TODO this should all be a little more seperated and handled by a texture system
 */
bool VulkanImageUtils::Create(VulkanImage& outImage) {
	VulkanData* d = &VulkanRenderer::m_VulkanData;

	stbi_uc* pixels = stbi_load("assets/textures/texture.jpg", &outImage.s_Width, &outImage.s_Height, &outImage.s_Channels, STBI_rgb_alpha);
	VkDeviceSize imageSize = (VkDeviceSize)outImage.s_Width * outImage.s_Height * 4;

	if (!pixels) {
		EN_ERROR("Failed to load image from disk.");
		return false;
	}

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	// Create staging buffer
	VulkanBufferUtils::CreateBuffer(imageSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		stagingBufferMemory,
		stagingBuffer);

	// Copy pixel data to staging buffer
	void* data;
	vkMapMemory(d->s_Device.s_LogicalDevice, stagingBufferMemory, 0, imageSize, 0, &data);
	Memory::Copy(data, pixels, (unsigned int)imageSize);
	vkUnmapMemory(d->s_Device.s_LogicalDevice, stagingBufferMemory);

	// Clean up pixel data
	stbi_image_free(pixels);

	if (!CreateImage(outImage.s_Width,
		outImage.s_Height,
		VK_FORMAT_R8G8B8A8_SRGB,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		outImage.s_Handle,
		outImage.s_Memory)) {
		return false;
	}
	
	// Transition the image layout and copy the staging buffers content to the vulkan image
	TransitionImageLayout(outImage.s_Handle, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	CopyBufferToImage(stagingBuffer, outImage.s_Handle, static_cast<uint32_t>(outImage.s_Width), static_cast<uint32_t>(outImage.s_Height));

	// Transition the layout again for the shader
	TransitionImageLayout(outImage.s_Handle, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	if (!CreateImageView(outImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT)) {
		EN_ERROR("Failed to create Image view.");
		return false;
	}

	if (!CreateTextureSampler(VulkanRenderer::m_VulkanData.s_Sampler)) {
		EN_ERROR("Failed to create texture image sampler.");
		return false;
	}

	// Cleanup
	vkDestroyBuffer(d->s_Device.s_LogicalDevice, stagingBuffer, d->s_Allocator);
	vkFreeMemory(d->s_Device.s_LogicalDevice, stagingBufferMemory, d->s_Allocator);
	return true;
}

bool VulkanImageUtils::CreateImage(uint32_t width,
	uint32_t height,
	VkFormat format,
	VkImageTiling tiling,
	VkImageUsageFlags usage,
	VkMemoryPropertyFlags properties,
	VkImage& image,
	VkDeviceMemory& imageMemory) {

	VulkanData* d = &VulkanRenderer::m_VulkanData;
	// Create the vulkan image
	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = static_cast<uint32_t>(width);
	imageInfo.extent.height = static_cast<uint32_t>(height);
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.format = format;
	imageInfo.tiling = tiling;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = usage;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.flags = 0; // Optional

	VK_CHECK(vkCreateImage(d->s_Device.s_LogicalDevice, &imageInfo, d->s_Allocator, &image));
	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(d->s_Device.s_LogicalDevice, image, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = VulkanBufferUtils::FindMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	if (vkAllocateMemory(d->s_Device.s_LogicalDevice, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate image memory!");
	}

	vkBindImageMemory(d->s_Device.s_LogicalDevice, image, imageMemory, 0);
	return true;
}

bool VulkanImageUtils::CreateImageView(VulkanImage& outImage, VkFormat format, VkImageAspectFlags aspectFlags) {
	VkImageViewCreateInfo viewInfo{};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = outImage.s_Handle;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = format;
	viewInfo.subresourceRange.aspectMask = aspectFlags;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;

	VK_CHECK(vkCreateImageView(VulkanRenderer::m_VulkanData.s_Device.s_LogicalDevice,
							   &viewInfo,
							   VulkanRenderer::m_VulkanData.s_Allocator,
							   &outImage.s_View));
	return true;
}

bool VulkanImageUtils::CreateTextureSampler(VkSampler& outSampler) {
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
	vkGetPhysicalDeviceProperties(VulkanRenderer::m_VulkanData.s_Device.s_PhysicalDevice, &properties);
	samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = 0.0f;

	VK_CHECK(vkCreateSampler(VulkanRenderer::m_VulkanData.s_Device.s_LogicalDevice,
							 &samplerInfo,
							 VulkanRenderer::m_VulkanData.s_Allocator,
							 &outSampler));
	return true;
}

bool VulkanImageUtils::CreateDepthResources(VulkanImage& depthImage) {
	VulkanData* d = &VulkanRenderer::m_VulkanData;
	VkFormat depthFormat = FindDepthFormat();

	// Create VkImage
	if (!CreateImage(d->s_Swapchain.s_Width,
		d->s_Swapchain.s_Height,
		depthFormat,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		depthImage.s_Handle,
		depthImage.s_Memory)) {
		EN_ERROR("Failed to create VkImage for depth image.");
		return false;
	}

	// Set the correct height and width of the image
	depthImage.s_Height = d->s_Swapchain.s_Height;
	depthImage.s_Width = d->s_Swapchain.s_Width;

	// Create VkImageView
	CreateImageView(depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);

	TransitionImageLayout(depthImage.s_Handle, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
	return true;
}

VkFormat VulkanImageUtils::FindSupportedFormat(DArray<VkFormat> candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {
	for (unsigned int i = 0; i < candidates.Size(); i++) {
		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(VulkanRenderer::m_VulkanData.s_Device.s_PhysicalDevice, candidates[i], &props);
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

VkFormat VulkanImageUtils::FindDepthFormat() {
	DArray<VkFormat> candidates;
	candidates.PushBack(VK_FORMAT_D32_SFLOAT);
	candidates.PushBack(VK_FORMAT_D32_SFLOAT_S8_UINT);
	candidates.PushBack(VK_FORMAT_D24_UNORM_S8_UINT);
	return FindSupportedFormat(
		candidates,
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
	);
}

void VulkanImageUtils::TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) {
	VkCommandBuffer commandBuffer = VulkanCommandbufferUtils::BeginSingleUseCommands();

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

		if (HasStencilComponent(format)) {
			barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
		}
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	}
	else {
		EN_ERROR("Unsupported layout transition in VulkanImageUtils::TransitionImageLayout.");
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

	VulkanCommandbufferUtils::EndSingleUseCommands(commandBuffer);
}

void VulkanImageUtils::CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
	VkCommandBuffer commandBuffer = VulkanCommandbufferUtils::BeginSingleUseCommands();

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
		image,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1,
		&region
	);

	VulkanCommandbufferUtils::EndSingleUseCommands(commandBuffer);
}

void VulkanImageUtils::Destroy(VulkanImage& image) {
	VulkanData* d = &VulkanRenderer::m_VulkanData;

	vkDestroyImageView(d->s_Device.s_LogicalDevice, image.s_View,d->s_Allocator);
	vkDestroyImage(d->s_Device.s_LogicalDevice, image.s_Handle, d->s_Allocator);
	vkFreeMemory(d->s_Device.s_LogicalDevice, image.s_Memory, d->s_Allocator);
} 