#pragma once

#include <vector>
#include <windows.h>
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
	bool createDebugMessenger(const VkInstance& instance, VkDebugUtilsMessengerEXT* debugMessenger);
public:
	VkAllocationCallbacks* m_Allocator = nullptr;
#ifdef _DEBUG
	VkDebugUtilsMessengerEXT m_DebugMessenger{};
#endif
private:
	VkInstance m_Handle{};
};

class VulkanSurface {
public:
	VulkanSurface() = delete;
	VulkanSurface(HWND windowHandle,
		const HINSTANCE& windowsInstance,
		const VulkanInstance& instance);
	~VulkanSurface();

private:
	const HINSTANCE m_WindowsInstance;
	const VulkanInstance& m_Instance;
public:
	VkSurfaceKHR m_Handle{};
};