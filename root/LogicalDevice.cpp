#include "LogicalDevice.h"
#include <set>

namespace vkn {

	LogicalDevice::LogicalDevice(vkn::PhysicalDevice *physicalDevice, 
		VkSurfaceKHR surface, 
		bool validationEnabled, 
		const std::vector<const char*> validationLayers) {

		physicalDevice = physicalDevice;

		vkn::QueueFamilyIndices indices = physicalDevice->findQueueFamilies(surface);

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

		float queuePriority = 1.0f;
		for (uint32_t queueFamily : uniqueQueueFamilies) {
			VkDeviceQueueCreateInfo queueCreateInfo{};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;

			queueCreateInfos.push_back(queueCreateInfo);
		}

		//Select the features from the PhysicalDevice we want to add to the Logical Device
		VkPhysicalDeviceFeatures deviceFeatures{};
		std::vector<const char*> extensions = physicalDevice->getDeviceExtensions();

		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.pQueueCreateInfos = queueCreateInfos.data();
		createInfo.queueCreateInfoCount = static_cast<uint32_t> (queueCreateInfos.size());
		createInfo.pEnabledFeatures = &deviceFeatures;
		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();
		if (validationEnabled) {
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();
		}
		else {
			createInfo.enabledLayerCount = 0;
		}

		if (vkCreateDevice(physicalDevice->getPhysicalDevice(), &createInfo, nullptr, &device) != VK_SUCCESS) {
			throw std::runtime_error("failed to create logical device!");
		}

		//Don't forget to add these back in somewhere in the main program
		//vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
		//vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
	}

	void LogicalDevice::getDeviceQueue(uint32_t queueFamilyIndex, uint32_t queueIndex, VkQueue *pQueue){
		vkGetDeviceQueue(this->device, queueFamilyIndex, queueIndex, pQueue);
	}

	LogicalDevice::~LogicalDevice() {
		vkDestroyDevice(device, nullptr);
	}

}
