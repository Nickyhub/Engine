#pragma once

#include <vector>

#include "vulkan/vulkan.h"

struct VulkanInstanceConfig {
	std::vector<const char*> extensions;
	std::vector<const char*> validationLayers;

	void populateWithDefaultValues();
};

class VulkanInstance {
public:
	VulkanInstance() = delete;
	VulkanInstance(VulkanInstanceConfig& instanceConfig);
	~VulkanInstance();
private:
	bool createDebugMessenger(VkInstance* instance, VkDebugUtilsMessengerEXT* debugMessenger);
private:
	VkInstance m_Handle;
};