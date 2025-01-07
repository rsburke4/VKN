#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#ifndef __LOGICAL_DEVICE_H__
#define __LOGICAL_DEVICE_H__
#include "PhysicalDevice.h"

namespace vkn {

	class LogicalDevice {
	public:
		//Fill out default constructor
		LogicalDevice() {}
		LogicalDevice(vkn::PhysicalDevice *device,
			VkSurfaceKHR surface,
			bool validationEnabled,
			const std::vector<const char*> validationLayers);
		~LogicalDevice();

		void getDeviceQueue(uint32_t queueFamilyIndex, uint32_t queueIndex, VkQueue *pQueue);
		VkDevice getDevice() { return device; }

	private:
		vkn::PhysicalDevice *physicalDevice;
		VkDevice device;
	};

}

#endif
