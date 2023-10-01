#include "VulkanInstance.hpp"
#include "VulkanUtils.hpp"
#include "Defines.hpp"

#include "core/Logger.hpp"
#include "core/Platform.hpp"
#include "core/String.hpp"
#include <vector>

// Callback for the Debug Messenger for validation layer errors
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSevertiy,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData) {
	if (messageSevertiy & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
		EN_ERROR("%s", pCallbackData->pMessage);
	}
	if (messageSevertiy & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
		EN_WARN("%s", pCallbackData->pMessage);
	}
	if (messageSevertiy & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) {
		EN_INFO("%s", pCallbackData->pMessage);
	}

	// Just output the error message
	return VK_FALSE;
}

void VulkanInstanceConfig::populateWithDefaultValues() {
	s_Extensions.emplace_back(Platform::getVulkanExtensions());
	s_Extensions.emplace_back(VK_KHR_SURFACE_EXTENSION_NAME);
	s_Extensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

	s_ValidationLayers.emplace_back("VK_LAYER_KHRONOS_validation");
}

VulkanInstance::VulkanInstance() {
	VulkanInstanceConfig config{};
	config.populateWithDefaultValues();
	create(config);
}

VulkanInstance::VulkanInstance(const VulkanInstanceConfig& instanceConfig) {
	create(instanceConfig);
}

void VulkanInstance::create(const VulkanInstanceConfig& instanceConfig) {
	// Get Application data
	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = instanceConfig.s_Name;
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = instanceConfig.s_Name;
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	// Fill out instance create info struct
	VkInstanceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	// Available extensions
	unsigned int extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
	std::vector<VkExtensionProperties> availableExtensions;
	availableExtensions.resize(extensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, availableExtensions.data());

	EN_INFO("Available extensions:");
	for (unsigned int i = 0; i < extensionCount; i++) {
		EN_INFO(availableExtensions[i].extensionName);
	}

	EN_DEBUG("Required extensions: ");
	// Output required extension
	for (unsigned int i = 0; i < instanceConfig.s_Extensions.size(); i++) {
		EN_DEBUG(instanceConfig.s_Extensions[i]);
	}

	createInfo.enabledExtensionCount = (uint32_t)instanceConfig.s_Extensions.size();
	createInfo.ppEnabledExtensionNames = &instanceConfig.s_Extensions[0];
	createInfo.enabledLayerCount = 0;

	// Enable validation layer support in debug mode
#ifdef _DEBUG
	// Get available validation layers
	std::vector<VkLayerProperties> availableValidationLayers;
	unsigned int validationLayerCount;
	vkEnumerateInstanceLayerProperties(&validationLayerCount, nullptr);
	availableValidationLayers.resize(validationLayerCount);
	vkEnumerateInstanceLayerProperties(&validationLayerCount, availableValidationLayers.data());

	// Check if the required layers are available
	bool found = false;
	for (unsigned int i = 0; i < validationLayerCount; i++) {
		for (unsigned int j = 0; j < instanceConfig.s_ValidationLayers.size(); j++) {
			if (String::StringCompare(availableValidationLayers[i].layerName, instanceConfig.s_ValidationLayers[j])) {
				found = true;
				break;
			}
		}
	}

	// If all have been found add them to the vkInstanceCreateInfo
	if (found) {
		createInfo.enabledLayerCount = (uint32_t)instanceConfig.s_ValidationLayers.size();
		createInfo.ppEnabledLayerNames = instanceConfig.s_ValidationLayers.data();
	}

#endif
	VK_CHECK(vkCreateInstance(&createInfo, m_Allocator, &this->m_Handle));
#ifdef _DEBUG
	// Can only create Debug Messenger after Instance has already been created
	createDebugMessenger(this->m_Handle, &m_DebugMessenger);
#endif
	EN_DEBUG("Vulkan instance created!");
}

VulkanInstance::~VulkanInstance() {
	EN_DEBUG("Destroying Vulkan Instance.");
	if (this->m_Handle) {
		vkDestroyInstance(this->m_Handle, m_Allocator);
	}
}

bool VulkanInstance::createDebugMessenger(const VkInstance& instance, VkDebugUtilsMessengerEXT* debugMessenger) {
	VkDebugUtilsMessengerCreateInfoEXT createInfo;
	createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = /*VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | */VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = debugCallback;

	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	VkResult result = func(instance, &createInfo, m_Allocator, debugMessenger);
	if (result != VK_SUCCESS) {
		EN_ERROR("Failed to create Debug Messenger.");
		return false;
	}

	EN_DEBUG("Vulkan debug messenger created.");
	return true;
}

