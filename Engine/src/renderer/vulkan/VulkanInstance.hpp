#pragma once

#include <vector>
#include <vulkan/vulkan.h>

struct VulkanInstanceConfig {
	std::vector<const char*> s_Extensions;
	std::vector<const char*> s_ValidationLayers;
	const char* s_Name;

	void populateWithDefaultValues();
};

class VulkanInstance {
public:
	VulkanInstance(const VulkanInstanceConfig& instanceConfig);
	VulkanInstance();
	~VulkanInstance();
	VkInstance getInternal() const { return m_Handle; }
private:
	void create(const VulkanInstanceConfig& instanceConfig);
	bool createDebugMessenger(VkInstance* instance, VkDebugUtilsMessengerEXT* debugMessenger);
public:
	VkAllocationCallbacks m_Allocator{};
#ifdef _DEBUG
	VkDebugUtilsMessengerEXT m_DebugMessenger{};
#endif
private:
	// Maybe implemeent allocator here
	VkInstance m_Handle;
};