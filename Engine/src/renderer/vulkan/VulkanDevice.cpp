#include "VulkanDevice.hpp"
#include "VulkanUtils.hpp"
#include "containers/Array.hpp"
#include "core/String.hpp"
#include "core/Logger.hpp"
#include "core/Platform.hpp"


VulkanDevice::VulkanDevice(const VulkanInstance& instance, bool enableSamplerAnisotropy, bool enableFillModeNonSolid) 
	: m_Instance(instance) {
	m_Surface = Platform::createVulkanSurface(instance.getInternal(), instance.m_Allocator);
	m_PhysicalDevice = selectPhysicalDevice();
	if (!m_PhysicalDevice) {
		EN_ERROR("No physical device present.");
	}

	// Logical device
	// Get unique indices in case queue families share indices
	Array<unsigned int, 4> familyIndices;
	familyIndices[0] = m_GraphicsQueueFamilyIndex;
	familyIndices[1] = m_PresentQueueFamilyIndex;
	familyIndices[2] = m_TransferQueueFamilyIndex;
	familyIndices[3] = m_ComputeQueueFamilyIndex;
	DArray<unsigned int> uniqueFamilyIndices;
	for (unsigned int i = 0; i < familyIndices.Size(); i++) {
		for (unsigned int j = 0; j < familyIndices.Size(); j++) {
			if (i != j && familyIndices[i] != familyIndices[j] && !uniqueFamilyIndices.Contains(familyIndices[i])) {
				uniqueFamilyIndices.PushBack(familyIndices[i]);
				break;
			}
		}
	}

	// Create and fill queue create infos
	DArray<VkDeviceQueueCreateInfo> queueCreateInfos;
	for (unsigned int i = 0; i < uniqueFamilyIndices.Size(); i++) {
		if (uniqueFamilyIndices[i] != -1) {
			VkDeviceQueueCreateInfo queueCreateInfo{};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = uniqueFamilyIndices[i];
			queueCreateInfo.queueCount = 1;
			float queuePriority = 1.0f;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.PushBack(queueCreateInfo);
		}
		else {
			EN_WARN("Graphics queue family index does not exist. No Graphics Queue created.");
		}
	}

	// TODO come back and specify the device features we want to use
	VkPhysicalDeviceFeatures deviceFeatures{};
	deviceFeatures.samplerAnisotropy = enableSamplerAnisotropy ? VK_TRUE : VK_FALSE;  // Request anistrophy
	deviceFeatures.fillModeNonSolid = enableFillModeNonSolid ? VK_TRUE : VK_FALSE;

	// Creating the logical device
	VkDeviceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.pQueueCreateInfos = queueCreateInfos.GetData();
	createInfo.queueCreateInfoCount = queueCreateInfos.Size();
	createInfo.pEnabledFeatures = &deviceFeatures;

	const char** extensionNames = (const char* [1])VK_KHR_SWAPCHAIN_EXTENSION_NAME;
	createInfo.enabledExtensionCount = 1;
	createInfo.ppEnabledExtensionNames = extensionNames;

	createInfo.enabledLayerCount = 0;
	createInfo.ppEnabledLayerNames = 0;

	VK_CHECK(vkCreateDevice(m_PhysicalDevice, &createInfo, &instance.m_Allocator, &m_LogicalDevice));

	// Get the queue handles after logical device creation
	vkGetDeviceQueue(m_LogicalDevice, m_GraphicsQueueFamilyIndex, 0, &m_GraphicsQueue);
	vkGetDeviceQueue(m_LogicalDevice, m_PresentQueueFamilyIndex, 0, &m_PresentQueue);
	vkGetDeviceQueue(m_LogicalDevice, m_TransferQueueFamilyIndex, 0, &m_TransferQueue);
	vkGetDeviceQueue(m_LogicalDevice, m_ComputeQueueFamilyIndex, 0, &m_ComputeQueue);

	// Create command pool for graphics queue
	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = m_GraphicsQueueFamilyIndex;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	VK_CHECK(vkCreateCommandPool(m_LogicalDevice, &poolInfo, &m_Instance.m_Allocator, &m_CommandPool));
	EN_DEBUG("Command pool created for graphics queue.");
	EN_DEBUG("Logical device created.");
}

VulkanDevice::~VulkanDevice() {
	vkDestroyCommandPool(m_LogicalDevice, m_CommandPool, nullptr);
	vkDestroyDevice(m_LogicalDevice, nullptr);
}

VkFormat VulkanDevice::findDepthFormat() const {
	// Format candidates
	DArray<VkFormat> candidates;
	candidates.PushBack(VK_FORMAT_D32_SFLOAT);
	candidates.PushBack(VK_FORMAT_D32_SFLOAT_S8_UINT);
	candidates.PushBack(VK_FORMAT_D24_UNORM_S8_UINT);

	unsigned int sizes[3] = {
		4,
		4,
		3 };

	unsigned int  flags = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
	for (int i = 0; i < 3; ++i) {
		VkFormatProperties properties;
		vkGetPhysicalDeviceFormatProperties(m_PhysicalDevice, candidates[i], &properties);

		if ((properties.linearTilingFeatures & flags) == flags) {
			return candidates[i];
		}
		else if ((properties.optimalTilingFeatures & flags) == flags) {
			return candidates[i];
		}
	}
	return candidates[0];
}

VkPhysicalDevice VulkanDevice::selectPhysicalDevice() {
	unsigned int deviceCount = 0;
	vkEnumeratePhysicalDevices(m_Instance.getInternal(),
							  &deviceCount,
							  nullptr);

	DArray<VkPhysicalDevice> physicalDevices;
	physicalDevices.Resize(deviceCount);
	VkPhysicalDeviceProperties properties;

	// Fill out requirements for the Device and choose it based on them
	VulkanPhysicalDeviceRequirements requirements{};
	requirements.s_ComputeQueue = true;
	requirements.s_GraphicsQueue = true;
	requirements.s_PresentQueue = true;
	requirements.s_DiscreteGPU = true;
	requirements.s_TransferQueue = true;
	requirements.s_SamplerAnisotropy = true;

	requirements.s_RequiredExtensions.PushBack(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

	if (deviceCount == 0) {
		EN_FATAL("Vulkan can not find a physical device. Cannot continue application.");
		return 0;
	}
	else if (deviceCount == 1) {
		vkEnumeratePhysicalDevices(m_Instance.getInternal(),
								   &deviceCount,
								   physicalDevices.GetData());
		vkGetPhysicalDeviceProperties(physicalDevices[0], &properties);
		if (!physicalDeviceMeetsRequirements(&physicalDevices[0], requirements)) {
			EN_FATAL("There is no device that meets the physical requirements. Cannot continue.");
			return 0;
		}
		EN_INFO("Physical device chosen: %s", properties.deviceName);
		return physicalDevices[0];
	}



	// Multiple devices are present. Choose the most suitable
	DArray<unsigned int> physicalDeviceScores;
	physicalDeviceScores.Fill(0);

	// Give every device a score
	for (unsigned int i = 0; i < deviceCount; i++) {
		vkGetPhysicalDeviceProperties(physicalDevices[i], &properties);
		VkPhysicalDeviceFeatures features{};
		vkGetPhysicalDeviceFeatures(physicalDevices[i], &features);

		if (!physicalDeviceMeetsRequirements(&physicalDevices[i], requirements)) {
			physicalDeviceScores[i] += 100;
			EN_INFO("Physical device %s does not meet the requiremtns. Skipping...", properties.deviceName);
			break;
		}

		physicalDeviceScores[i] += properties.limits.maxImageDimension2D;
		if (!features.geometryShader) {
			EN_WARN("Geometry shader not present on device: %s", properties.deviceName);
			physicalDeviceScores[i] -= 10;
		}
	}

	// Choose the device with the highest score
	unsigned int score = 0;
	unsigned int deviceIndex = 0;
	for (unsigned int i = 0; i < deviceCount; i++) {
		if (physicalDeviceScores[i] >= score) {
			score = physicalDeviceScores[i];
			deviceIndex = i;
		}
	}
	if (physicalDevices[deviceIndex]) {
		return physicalDevices[deviceIndex];
	}
	EN_FATAL("No physical device was picked. Cannot continue.");
	return 0;
}

bool VulkanDevice::physicalDeviceMeetsRequirements(const VkPhysicalDevice* device, const VulkanPhysicalDeviceRequirements& requirements) {
	// Check if the device is a discrete GPU
	VkPhysicalDeviceProperties properties;
	vkGetPhysicalDeviceProperties(*device, &properties);

	if ((properties.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) && requirements.s_DiscreteGPU) {
		EN_INFO("Device %s is not a discrete GPU. Skipping.", properties.deviceName);
		return false;
	}

	unsigned int queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(*device, &queueFamilyCount, nullptr);

	DArray<VkQueueFamilyProperties> queueFamilies;
	queueFamilies.Resize(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(*device, &queueFamilyCount, queueFamilies.GetData());

	int minTransferScore = 255;

	// Local indices to check them later
	unsigned int graphicsIndex = -1, presentIndex = -1, computeIndex = -1, transferIndex = -1;

	for (unsigned int i = 0; i < queueFamilyCount; i++) {
		// Graphics queue?
		int currentTransferScore = 0;
		int flags = queueFamilies[i].queueFlags;
		if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			graphicsIndex = i;
			VkBool32 supportsPresent = VK_FALSE;
			vkGetPhysicalDeviceSurfaceSupportKHR(*device,
												 i,
												 m_Surface,
												 &supportsPresent);
			if (supportsPresent) {
				presentIndex = i;
				++currentTransferScore;
			}
			++currentTransferScore;
		}
		// Compute queue?
		if (queueFamilies[i].queueFlags & VK_QUEUE_COMPUTE_BIT) {
			computeIndex = i;
			++currentTransferScore;
		}
		// Transfer queue?
		if (queueFamilies[i].queueFlags & VK_QUEUE_TRANSFER_BIT) {
			// Prefer dedicated transfer queue
			// Take the index if it is the current lowest. This increases the 
			// liklihood that it is a dedicated transfer queue.
			if (currentTransferScore <= minTransferScore) {
				minTransferScore = currentTransferScore;
				transferIndex = i;
			}
		}
	}

	// It is possible that no present queue has been found with the same graphics queue index
	// Iterate again and choose the first present queue found
	if (presentIndex == -1) {
		for (unsigned int i = 0; i < queueFamilyCount; i++) {
			VkBool32 supportsPresent = VK_FALSE;
			vkGetPhysicalDeviceSurfaceSupportKHR(*device,
												 i,
												 m_Surface,
												 &supportsPresent);
			if (supportsPresent) {
				presentIndex = i;

				if (presentIndex != graphicsIndex) {
					EN_WARN("Different queue index used for present vs graphics: %u.", i);
				}
				break;
			}
		}
	}

	if ((!requirements.s_GraphicsQueue || requirements.s_GraphicsQueue && graphicsIndex != -1) &&
		(!requirements.s_PresentQueue || requirements.s_PresentQueue && presentIndex != -1) &&
		(!requirements.s_ComputeQueue || requirements.s_ComputeQueue && computeIndex != -1) &&
		(!requirements.s_TransferQueue || requirements.s_TransferQueue && transferIndex != -1)) {

		EN_INFO("Device meets queue requirements. ");

		// Print info about the device
		EN_INFO("Graphics queue index: %u.", graphicsIndex);
		EN_INFO("Present queue index: %u.", presentIndex);
		EN_INFO("Compute queue index: %u.", computeIndex);
		EN_INFO("Transfer queue index: %u.", transferIndex);

		// Copy over the indices from local variables to the VulkanDevice
		m_GraphicsQueueFamilyIndex = graphicsIndex;
		m_PresentQueueFamilyIndex = presentIndex;
		m_ComputeQueueFamilyIndex = computeIndex;
		m_TransferQueueFamilyIndex = transferIndex;

		// Fill the struct with physical device features, properties and memory infos
		vkGetPhysicalDeviceFeatures(*device, &m_Features);
		vkGetPhysicalDeviceProperties(*device, &m_Properties);
		vkGetPhysicalDeviceMemoryProperties(*device, &m_Memory);

		// Output some more info
		;
		EN_INFO(
			"GPU Driver version: %d.%d.%d",
			VK_VERSION_MAJOR(m_Properties.driverVersion),
			VK_VERSION_MINOR(m_Properties.driverVersion),
			VK_VERSION_PATCH(m_Properties.driverVersion));

		// Vulkan API version.
		EN_INFO(
			"Vulkan API version: %d.%d.%d",
			VK_VERSION_MAJOR(m_Properties.apiVersion),
			VK_VERSION_MINOR(m_Properties.apiVersion),
			VK_VERSION_PATCH(m_Properties.apiVersion));

		// Memory info
		for (unsigned int j = 0; j < m_Memory.memoryHeapCount; ++j) {
			float memory_size_gib = (((float)m_Memory.memoryHeaps[j].size) / 1024.0f / 1024.0f / 1024.0f);
			if (m_Memory.memoryHeaps[j].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {
				EN_INFO("Local GPU memory: %.2f GiB", memory_size_gib);
			}
			else {
				EN_INFO("Shared System memory: %.2f GiB", memory_size_gib);
			}
		}

		// Check if extensions are available
		unsigned int availableExtensionCount = 0;
		DArray<VkExtensionProperties> availableDeviceExtensions;
		vkEnumerateDeviceExtensionProperties(*device, 0, &availableExtensionCount, 0);
		availableDeviceExtensions.Resize(availableExtensionCount);
		vkEnumerateDeviceExtensionProperties(*device, 0, &availableExtensionCount, availableDeviceExtensions.GetData());

		for (unsigned int i = 0; i < requirements.s_RequiredExtensions.Size(); i++) {
			bool found = false;
			for (unsigned int j = 0; j < availableExtensionCount; j++) {
				if (String::StringCompare(availableDeviceExtensions[j].extensionName, requirements.s_RequiredExtensions[i])) {
					found = true;
					break;
				}
			}
			if (!found) {
				EN_INFO("Required device extension %s not found. Skipping.", requirements.s_RequiredExtensions[i]);
				return false;
			}
		}


		if (!querySwapchainSupport(device)) {
			EN_ERROR("Device does not meet the swapchain support requirements. Skipping.");
			return false;
		}
		// Check sampler anisotropy
		if (requirements.s_SamplerAnisotropy && !m_Features.samplerAnisotropy) {
			EN_INFO("Device does not support samplerAnisotropy, skipping.");
			return false;
		}
		return true;
	}
	else {
		EN_ERROR("Device does not meet queue requirements. Skipping.");
		return false;
	}
	return false;
}

bool VulkanDevice::querySwapchainSupport(const VkPhysicalDevice* device) {
	// Create information structs to be filled
	VkSurfaceCapabilitiesKHR capabilities;
	//DArray<VkSurfaceFormatKHR> formats;
	//DArray<VkPresentModeKHR> presentModes;

	VulkanDeviceSwapchainSupportInfo* supportInfo = &m_SwapchainSupportInfo;

	// Query surface capabilities
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(*device, m_Surface, &capabilities);

	// Query formats
	unsigned int formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(*device, m_Surface, &formatCount, 0);
	// Check if formats are available
	if (formatCount != 0) {
		supportInfo->s_Formats.Resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(*device, m_Surface, &formatCount, supportInfo->s_Formats.GetData());
	} else {
		EN_ERROR("Physical device has no surface formats available. Skipping.");
		return false;
	}

	// Query present modes
	unsigned int presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(*device, m_Surface, &presentModeCount, 0);
	// Check if present modes are available
	if (presentModeCount != 0) {
		supportInfo->s_PresentModes.Resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(*device, m_Surface, &presentModeCount, supportInfo->s_PresentModes.GetData());
	}
	else {
		EN_ERROR("Physical device has no surface present modes available. Skipping.");
		return false;
	}
	supportInfo->s_FormatCount = formatCount;
	supportInfo->s_PresentModeCount = presentModeCount;
	// Set capabilities
	supportInfo->s_Capabilities = capabilities;

	EN_INFO("Vulkan swapchain support is sufficient and data has been copied to vulkan data.");
	return true;
}
