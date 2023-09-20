#pragma once

#include <vulkan/vulkan.h>

#define VK_CHECK(expression)								\
	if(expression != VK_SUCCESS) {							\
		EN_ERROR("Error executing %s", ##expression);		\
		__debugbreak();									\
	}

const char* VulkanResultString(VkResult result, bool getExtended);