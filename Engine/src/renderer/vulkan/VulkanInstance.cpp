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


bool VulkanInstance::Create(VkInstance* outInstance) {
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

	// Fill array with required extensions
	DArray<const char*> extensions;
	extensions.PushBack(Platform::GetVulkanExtensions());
	extensions.PushBack(VK_KHR_SURFACE_EXTENSION_NAME);
	extensions.PushBack(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

	// Available extensions
	unsigned int extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
	DArray<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, availableExtensions.GetData());
	
	EN_INFO("Available extensions:");
	for (unsigned int i = 0; i < extensionCount; i++) {
		EN_INFO(availableExtensions[i].extensionName);
	}


	EN_DEBUG("Required extensions: ");
	// Output required extension
	for (unsigned int i = 0; i < extensions.Size(); i++) {
		EN_DEBUG(extensions[i]);
	}

	createInfo.enabledExtensionCount = extensions.Size();
	createInfo.ppEnabledExtensionNames = &extensions[0];
	createInfo.enabledLayerCount = 0;

// Enable validation layer support in debug mode
#ifdef _DEBUG
	// Create required validation layers
	DArray<const char*> requiredValidationLayers;
	requiredValidationLayers.PushBack("VK_LAYER_KHRONOS_validation");

	// Get available validation layers
	DArray<VkLayerProperties> availableValidationLayers;
	unsigned int validationLayerCount;
	vkEnumerateInstanceLayerProperties(&validationLayerCount, nullptr);
	availableValidationLayers.Resize(validationLayerCount);
	vkEnumerateInstanceLayerProperties(&validationLayerCount, availableValidationLayers.GetData());

	// Check if the required layers are available
	bool found = false;
	for (unsigned int i = 0; i < validationLayerCount; i++) {
		for (unsigned int j = 0; j < requiredValidationLayers.Size(); j++) {
			if (String::StringCompare(availableValidationLayers[i].layerName, requiredValidationLayers[j])) {
				found = true;
			}
		}
	}

	// If all have been found add them to the vkInstanceCreateInfo
	if (found) {
		createInfo.enabledLayerCount = requiredValidationLayers.Size();
		createInfo.ppEnabledLayerNames = requiredValidationLayers.GetData();
	}

#endif

	VK_CHECK(vkCreateInstance(&createInfo, VulkanRenderer::m_VulkanData.s_Allocator, outInstance));

#ifdef _DEBUG
	// Can only create Debug Messenger after Instance has already been created
	CreateDebugMessenger(outInstance, &VulkanRenderer::m_VulkanData.s_DebugMessenger);
#endif

	EN_DEBUG("Vulkan instance created!");
	return true;
}

void VulkanInstance::Destroy(VkInstance* instance) {
	EN_DEBUG("Destroying Vulkan Instance.");
	if (instance) {
		vkDestroyInstance(*instance, VulkanRenderer::m_VulkanData.s_Allocator);
	}

}

bool VulkanInstance::CreateDebugMessenger(VkInstance* instance, VkDebugUtilsMessengerEXT* debugMessenger) {
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

