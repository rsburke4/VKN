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
		~LogicalDevice() {}

	private:
		vkn::PhysicalDevice *physicalDevice;
		VkDevice device;
	};

}

#endif __LOGICAL_DEVICE_H__
