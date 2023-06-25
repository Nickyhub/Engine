#pragma once

#include "vulkan/vulkan.h"

class VulkanInstance {
public:
	VulkanInstance() = delete;
	static bool Create(VkInstance* outInstance);
	static void Destroy(VkInstance* instance);

private:
	static bool CreateDebugMessenger(VkInstance* instance, VkDebugUtilsMessengerEXT* debugMessenger);
};