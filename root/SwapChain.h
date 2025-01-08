#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#ifndef __SWAP_CHAIN_H__
#define __SWAP_CHAIN_H__

#include "PhysicalDevice.h"
#include "LogicalDevice.h"

namespace vkn {
	class SwapChain {
	public:
		SwapChain() {}
		SwapChain(PhysicalDevice* device, LogicalDevice *lDevice, VkSurfaceKHR surface, GLFWwindow* window);
		~SwapChain() { vkDestroySwapchainKHR(logicalDevice->getDevice(), swapChain, nullptr); }

		VkExtent2D getExtent() { return extent; }
		VkSurfaceFormatKHR getFormat() { return surfaceFormat; }
		VkPresentModeKHR getPresentMode() { return presentMode; }
		VkSwapchainKHR getSwapChain() { return swapChain; }


	private:
		void chooseSwapSurfaceFormat();
		void chooseSwapExtent();
		void chooseSwapPresentMode();

		vkn::PhysicalDevice *physicalDevice;
		vkn::LogicalDevice* logicalDevice;
		vkn::SwapChainSupportDetails swapChainSupport;
		VkSwapchainKHR swapChain;
		std::vector<VkImage> images;
		VkExtent2D extent;
		VkSurfaceFormatKHR surfaceFormat;
		VkPresentModeKHR presentMode;
		GLFWwindow* window;
	};
}

#endif