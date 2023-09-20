#pragma once
#include <vulkan/vulkan.h>
#include "VulkanInstance.hpp"

#include "containers/DArray.hpp"
#include "Defines.hpp"

struct VulkanDeviceSwapchainSupportInfo {
	unsigned int s_FormatCount = 0;
	DArray<VkSurfaceFormatKHR> s_Formats;

	unsigned int s_PresentModeCount = 0;
	DArray<VkPresentModeKHR> s_PresentModes;

	VkSurfaceCapabilitiesKHR s_Capabilities{};
};

struct VulkanPhysicalDeviceRequirements {
	bool s_GraphicsQueue = false;
	bool s_PresentQueue = false;
	bool s_TransferQueue = false;
	bool s_ComputeQueue = false;
	DArray<const char*> s_RequiredExtensions;

	bool s_SamplerAnisotropy = false;
	bool s_DiscreteGPU = false;
};

class VulkanDevice {
public:
	VulkanDevice() = delete;
	VulkanDevice(const VulkanInstance& instance, bool enableSamplerAnisotropy, bool enableFillModeNonSolid);
	~VulkanDevice();
	VkFormat findDepthFormat() const;
private:
	bool querySwapchainSupport(const VkPhysicalDevice* device);
	VkPhysicalDevice selectPhysicalDevice();
	bool physicalDeviceMeetsRequirements(const VkPhysicalDevice* device, const VulkanPhysicalDeviceRequirements& requirements);
	
public:
	VkDevice m_LogicalDevice = nullptr;
	VkPhysicalDevice m_PhysicalDevice = nullptr;
	VkSurfaceKHR m_Surface;
	VulkanDeviceSwapchainSupportInfo m_SwapchainSupportInfo;

	unsigned int m_GraphicsQueueFamilyIndex = INVALID_ID;
	unsigned int m_ComputeQueueFamilyIndex = INVALID_ID;
	unsigned int m_PresentQueueFamilyIndex = INVALID_ID;
	unsigned int m_TransferQueueFamilyIndex = INVALID_ID;

	VkQueue m_PresentQueue = nullptr;
	VkQueue m_GraphicsQueue = nullptr;
	VkCommandPool m_CommandPool = nullptr;
private:
	VkPhysicalDeviceFeatures m_Features{};
	VkPhysicalDeviceProperties m_Properties{};
	VkPhysicalDeviceMemoryProperties m_Memory{};

	VkQueue m_ComputeQueue = nullptr;
	VkQueue m_TransferQueue = nullptr;

	const VulkanInstance& m_Instance;
};