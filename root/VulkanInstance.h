/*
Reagan Burke
Code created by following the guides on https://vulkan-tutorial.com
*/

#include <vector>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#ifndef __VULKAN_INSTANCE_H__
#define __VULKAN_INSTANCE_H__
#include <iostream>

namespace vkn {

	class VulkanInstance {
	public:
		VulkanInstance() : VulkanInstance(false) {}
		VulkanInstance(bool validation);
		VulkanInstance(bool validation, VkApplicationInfo appInfo);
		~VulkanInstance();

		VkInstance getInstance() { return instance; }


	private:
		//Encapsules struct generation
		bool checkValidationLayerSupport();
		void getRequiredExtensions();
		void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& debugInfo);
		void populateCreateInfo(VkInstanceCreateInfo&, VkApplicationInfo& createInfo);
		void populateApplicationInfo(VkApplicationInfo& appInfo);

		//Debugging setup
		void setupDebugMessenger();
		VkResult CreateDebugUtilsMessengerEXT(const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
			const VkAllocationCallbacks* pAllocator);
		void DestroyDebugUtilMessengerEXT(const VkAllocationCallbacks* pAllocator);

		static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
			VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT messageType,
			const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
			void* pUserData) {

			std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

			return false;
		}

		bool enabledValidationLayers;
		const std::vector<const char*> validationLayers;
		std::vector<const char*> extensions;
		VkInstance instance;
		VkDebugUtilsMessengerEXT debugMessenger;
	};

} 

#endif