#pragma once
#include <vulkan/vulkan.h>
#include "VulkanInstance.hpp"

#include <vector>
#include "Defines.hpp"

struct VulkanDeviceSwapchainSupportInfo {
	VulkanDeviceSwapchainSupportInfo() {}
	unsigned int s_FormatCount = 0;
	std::vector<VkSurfaceFormatKHR> s_Formats;

	unsigned int s_PresentModeCount = 0;
	std::vector<VkPresentModeKHR> s_PresentModes;

	VkSurfaceCapabilitiesKHR s_Capabilities{};
};

struct VulkanPhysicalDeviceRequirements {
	bool s_GraphicsQueue = false;
	bool s_PresentQueue = false;
	bool s_TransferQueue = false;
	bool s_ComputeQueue = false;
	std::vector<const char*> s_RequiredExtensions;

	bool s_SamplerAnisotropy = false;
	bool s_DiscreteGPU = false;
};

class VulkanDevice {
public:
	VulkanDevice() = delete;
	VulkanDevice(const VulkanSurface& surface, const VulkanInstance& instance, bool enableSamplerAnisotropy, bool enableFillModeNonSolid);
	~VulkanDevice();
	VkFormat findDepthFormat() const;
private:
	bool querySwapchainSupport(const VkPhysicalDevice* device);
	VkPhysicalDevice selectPhysicalDevice();
	bool physicalDeviceMeetsRequirements(const VkPhysicalDevice* device, const VulkanPhysicalDeviceRequirements& requirements);
	
private:
	const VulkanInstance& m_Instance; // needs to be initialized before the physical device but still private
	VkPhysicalDeviceFeatures m_Features{};
	VkPhysicalDeviceProperties m_Properties{};
	VkPhysicalDeviceMemoryProperties m_Memory{};

	VkQueue m_ComputeQueue = nullptr;
	VkQueue m_TransferQueue = nullptr;
public:
	const VulkanSurface& m_Surface;
	VkDevice m_LogicalDevice = nullptr;
	VulkanDeviceSwapchainSupportInfo m_SwapchainSupportInfo;

	unsigned int m_GraphicsQueueFamilyIndex = INVALID_ID;
	unsigned int m_ComputeQueueFamilyIndex = INVALID_ID;
	unsigned int m_PresentQueueFamilyIndex = INVALID_ID;
	unsigned int m_TransferQueueFamilyIndex = INVALID_ID;

	const VkPhysicalDevice m_PhysicalDevice;

	VkQueue m_PresentQueue = nullptr;
	VkQueue m_GraphicsQueue = nullptr;
	VkCommandPool m_CommandPool = nullptr;
};