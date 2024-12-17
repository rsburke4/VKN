#include "Instance.h"
#include <stdexcept>

using namespace vkn;


void VulkanInstance::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
	createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = debugCallback; //The function we made above
	createInfo.pUserData = nullptr; //Optional
}

void VulkanInstance::populateCreateInfo(VkInstanceCreateInfo& createInfo, VkApplicationInfo& appInfo) {
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;
	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();
	createInfo.enabledLayerCount = 0;
	createInfo.pNext = nullptr;
}

void VulkanInstance::populateApplicationInfo(VkApplicationInfo& appInfo) {
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Hello Triangle";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "No Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;
}

void VulkanInstance::getRequiredExtensions() {
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector<const char*>requiredExtensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
	if (enabledValidationLayers) {
		requiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}
	extensions = requiredExtensions;
}

//Vulkan Instance with custom application info
VulkanInstance::VulkanInstance(bool validation, VkApplicationInfo appInfo) {
	enabledValidationLayers = validation;
	if (enabledValidationLayers && !checkValidationLayerSupport()) {
		throw std::runtime_error("validation layers requested, but not supported");
	}
	getRequiredExtensions();

	VkInstanceCreateInfo createInfo{};
	populateCreateInfo(createInfo, appInfo);

	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
	if (enabledValidationLayers) {
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();

		populateDebugMessengerCreateInfo(debugCreateInfo);
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
	}

	//Use createInfo to generateInstance
	if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
		throw std::runtime_error("failed to create instance!");
	}
}

//Validation Layers? Application Info? 
VulkanInstance::VulkanInstance(bool validation) {
	//Create Vulkan Info here
	// 
	//First determine if validation layers are turned on.
	enabledValidationLayers = validation;
	if (enabledValidationLayers && !checkValidationLayerSupport()) {
		throw std::runtime_error("validation layers requested, but not supported");
	}
	getRequiredExtensions();

	//Default values for a struct that could be different if passed in from a
	//Non-default constructor
	VkApplicationInfo appInfo{};
	populateApplicationInfo(appInfo);

	//NOT OPTIONAL struct for creating global extention scope
	VkInstanceCreateInfo createInfo{};
	populateCreateInfo(createInfo, appInfo);

	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
	if (enabledValidationLayers) {
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();

		populateDebugMessengerCreateInfo(debugCreateInfo);
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
	}

	//Use createInfo to generateInstance
	if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
		throw std::runtime_error("failed to create instance!");
	}

}

VulkanInstance::~VulkanInstance() {
	//Destroy vulkan instance here
	if (enabledValidationLayers) {
		DestroyDebugUtilMessengerEXT(nullptr);
	}
	vkDestroyInstance(instance, nullptr);
}

bool VulkanInstance::checkValidationLayerSupport() {
	//Get the total number of instancelayers
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	//Same function, but this time passing in a vector with preallocated size
	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	//For each layerName in validationLayers
	for (const char* layerName : validationLayers) {
		bool layerFound = false;

		//For each layerProperty in availableLayers
		for (const auto& layerProperties : availableLayers) {
			if (strcmp(layerName, layerProperties.layerName) == 0) {
				layerFound = true;
				break;
			}
		}
		if (!layerFound) {
			return false;
		}
	}
	return true;
}

VkResult VulkanInstance::CreateDebugUtilsMessengerEXT(const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
	const VkAllocationCallbacks* pAllocator) {

	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance,
		"vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr) {
		return func(instance, pCreateInfo, pAllocator, &debugMessenger);
	}
	else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}

}

void VulkanInstance::setupDebugMessenger() {
	if (!enabledValidationLayers) return;
	VkDebugUtilsMessengerCreateInfoEXT createInfo;
	populateDebugMessengerCreateInfo(createInfo);
	if (CreateDebugUtilsMessengerEXT(&createInfo, nullptr));
}

void VulkanInstance::DestroyDebugUtilMessengerEXT(const VkAllocationCallbacks* pAllocator) {
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance,
		"vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr) {
		func(instance, debugMessenger, pAllocator);
	}
}