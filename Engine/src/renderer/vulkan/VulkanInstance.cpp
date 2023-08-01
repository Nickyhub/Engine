#include "VulkanInstance.hpp"
#include "VulkanRenderer.hpp"
#include "Defines.hpp"

#include "core/Application.hpp"
#include "core/Logger.hpp"
#include "core/Platform.hpp"
#include "core/String.hpp"

#include "containers/DArray.hpp"

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
	extensions.emplace_back(Platform::GetVulkanExtensions());
	extensions.emplace_back(VK_KHR_SURFACE_EXTENSION_NAME);
	extensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

	validationLayers.emplace_back("VK_LAYER_KHRONOS_validation");
}

VulkanInstance::VulkanInstance(VulkanInstanceConfig& instanceConfig) {
// Get Application data
	ApplicationConfig aConfig = Application::GetConfig();
	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = aConfig.Name;
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = aConfig.Name;
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	// Fill out instance create info struct
	VkInstanceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	// Available extensions
	unsigned int extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
	DArray<VkExtensionProperties> availableExtensions;
	availableExtensions.Resize(extensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, availableExtensions.GetData());
	
	EN_INFO("Available extensions:");
	for (unsigned int i = 0; i < extensionCount; i++) {
		EN_INFO(availableExtensions[i].extensionName);
	}


	EN_DEBUG("Required extensions: ");
	// Output required extension
	for (unsigned int i = 0; i < instanceConfig.extensions.size(); i++) {
		EN_DEBUG(instanceConfig.extensions[i]);
	}

	createInfo.enabledExtensionCount = instanceConfig.extensions.size();
	createInfo.ppEnabledExtensionNames = &instanceConfig.extensions[0];
	createInfo.enabledLayerCount = 0;

// Enable validation layer support in debug mode
#ifdef _DEBUG
	// Get available validation layers
	DArray<VkLayerProperties> availableValidationLayers;
	unsigned int validationLayerCount;
	vkEnumerateInstanceLayerProperties(&validationLayerCount, nullptr);
	availableValidationLayers.Resize(validationLayerCount);
	vkEnumerateInstanceLayerProperties(&validationLayerCount, availableValidationLayers.GetData());

	// Check if the required layers are available
	bool found = false;
	for (unsigned int i = 0; i < validationLayerCount; i++) {
		for (unsigned int j = 0; j < instanceConfig.validationLayers.size(); j++) {
			if (String::StringCompare(availableValidationLayers[i].layerName, instanceConfig.validationLayers[j])) {
				found = true;
			}
		}
	}

	// If all have been found add them to the vkInstanceCreateInfo
	if (found) {
		createInfo.enabledLayerCount = instanceConfig.validationLayers.size();
		createInfo.ppEnabledLayerNames = instanceConfig.validationLayers.data();
	}

#endif
	VK_CHECK(vkCreateInstance(&createInfo, VulkanRenderer::m_VulkanData.s_Allocator, &this->m_Handle));
#ifdef _DEBUG
	// Can only create Debug Messenger after Instance has already been created
	createDebugMessenger(&this->m_Handle, &VulkanRenderer::m_VulkanData.s_DebugMessenger);
#endif
	EN_DEBUG("Vulkan instance created!");
}

VulkanInstance::~VulkanInstance() {
EN_DEBUG("Destroying Vulkan Instance.");
	if (this->m_Handle) {
		vkDestroyInstance(this->m_Handle, VulkanRenderer::m_VulkanData.s_Allocator);
	}
}

bool VulkanInstance::createDebugMessenger(VkInstance* instance, VkDebugUtilsMessengerEXT* debugMessenger) {
	VkDebugUtilsMessengerCreateInfoEXT createInfo;
	createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = /*VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | */VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = debugCallback;

	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(*instance, "vkCreateDebugUtilsMessengerEXT");
	VkResult result = func(*instance, &createInfo, VulkanRenderer::m_VulkanData.s_Allocator, debugMessenger);
	if (result != VK_SUCCESS) {
		EN_ERROR("Failed to create Debug Messenger.");
		return false;
	}

	EN_DEBUG("Vulkan debug messenger created.");
	return true;
}

