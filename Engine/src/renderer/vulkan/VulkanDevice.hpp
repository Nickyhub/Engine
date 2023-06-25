#pragma once
#include <vulkan/vulkan.h>
#include "containers/DArray.hpp"
#include "Defines.hpp"
#include "VulkanSwapchain.hpp"

struct VulkanDeviceSwapchainSupportInfo {
	unsigned int s_FormatCount = 0;
	DArray<VkSurfaceFormatKHR> s_Formats;

	unsigned int s_PresentModeCount = 0;
	DArray<VkPresentModeKHR> s_PresentModes;

	VkSurfaceCapabilitiesKHR s_Capabilities{};
};

struct  VulkanDevice {
	VkPhysicalDevice s_PhysicalDevice = nullptr;
	VkDevice s_LogicalDevice = nullptr;

	VkPhysicalDeviceFeatures s_Features{};
	VkPhysicalDeviceProperties s_Properties{};
	VkPhysicalDeviceMemoryProperties s_Memory{};

	VulkanDeviceSwapchainSupportInfo s_SwapchainSupportInfo;

	VkCommandPool s_CommandPool = nullptr;

	VkQueue s_GraphicsQueue = nullptr;
	VkQueue s_PresentQueue = nullptr;
	VkQueue s_ComputeQueue = nullptr;
	VkQueue s_TransferQueue = nullptr;
	unsigned int s_GraphicsQueueFamilyIndex = INVALID_ID;
	unsigned int s_ComputeQueueFamilyIndex = INVALID_ID;
	unsigned int s_PresentQueueFamilyIndex = INVALID_ID;
	unsigned int s_TransferQueueFamilyIndex = INVALID_ID;
};

struct VulkanPhysicalDeviceRequirements {
	bool s_GraphicsQueue = false;
	bool s_PresentQueue = false;
	bool s_TransferQueue = false;
	bool s_ComputeQueue = false;

	//std::vector < const char*> s_RequiredExtensions;
	DArray<const char*> s_RequiredExtensions;

	bool s_SamplerAnisotropy = false;
	bool s_DiscreteGPU = false;
};

class VulkanDeviceUtils {
public:
	static bool Create(VulkanDevice* outDevice);
	static void Destroy(VulkanDevice* device);
	
private:
	static bool QuerySwapchainSupport(const VkPhysicalDevice* device);
	static VkPhysicalDevice SelectPhysicalDevice();
	static bool PhysicalDeviceMeetsRequirements(const VkPhysicalDevice* device, const VulkanPhysicalDeviceRequirements& requirements);
};