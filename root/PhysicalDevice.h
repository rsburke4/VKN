#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <optional>
#include <vector>
#include "VulkanInstance.h"

#ifndef __PHYSICAL_DEVICE_H__
#define __PHYSICAL_DEVICE_H__

namespace vkn {

	struct QueueFamilyIndices {
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;
		bool isComplete() {
			return graphicsFamily.has_value() && presentFamily.has_value();
		}
	};

	struct SwapChainSupportDetails {
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	class PhysicalDevice {
	public:
		PhysicalDevice() { instance = NULL; physicalDevice = VK_NULL_HANDLE; }
		PhysicalDevice(VulkanInstance* vknInstance, VkSurfaceKHR surface);
		~PhysicalDevice() {}

		VkPhysicalDevice getPhysicalDevice() { return physicalDevice; }
		QueueFamilyIndices findQueueFamilies(VkSurfaceKHR);
		SwapChainSupportDetails querySwapChainSupport(VkSurfaceKHR);

	private:
		bool isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface);
		bool checkDeviceExtensionSupport(VkPhysicalDevice device);
		QueueFamilyIndices findQueueFamilies(VkPhysicalDevice, VkSurfaceKHR);
		SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);

		VkPhysicalDevice physicalDevice;
		VulkanInstance* instance;
		std::vector<const char*> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
		SwapChainSupportDetails swapChainSupport;
	};

}

#endif