#include "SwapChain.h"
#include <algorithm>

namespace vkn {

SwapChain::SwapChain(PhysicalDevice *device, LogicalDevice *lDevice, VkSurfaceKHR surface, GLFWwindow* win) {
	physicalDevice = device;
	logicalDevice = lDevice;
	window = win;

	swapChainSupport = physicalDevice->querySwapChainSupport(surface);

	//Initializer functions for swap chain details
	//Keeps things clean
	chooseSwapSurfaceFormat();
	chooseSwapPresentMode();
	chooseSwapExtent();

	//The minimum number of images for the swap chain to function (plus one for no wait times)
	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
	//But capped to the max, unless there is no max (0)
	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = surface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	//Below indicates that we are rendering directly to the swap chain
	//Opposed to using the swap chain as an intermediate processing buffer
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	vkn::QueueFamilyIndices indices = device->findQueueFamilies(surface);
	uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

	//If the graphics and presentation queue families are different
	//Then charing needs to be active
	if (indices.graphicsFamily != indices.presentFamily) {
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else {
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0; //optional
		createInfo.pQueueFamilyIndices = nullptr;
	}

	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.clipped = VK_TRUE; //If the window is obscured, these pixels are ignored
	//createInfo.oldSwapchain = VK_NULL_HANDLE; //This is a link to a depricated swap chain, if this swap chain is an updated one

	if (vkCreateSwapchainKHR(logicalDevice->getDevice(), &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
		throw std::runtime_error("failed to create swap chain!");
	}

	//Store handles to swap chain images in the member vector
	vkGetSwapchainImagesKHR(logicalDevice->getDevice(), swapChain, &imageCount, nullptr);
	images.resize(imageCount);
	vkGetSwapchainImagesKHR(logicalDevice->getDevice(), swapChain, &imageCount, images.data());
}

//Selects a format for the images based on a preferred image format for the surface.
//We would like to select 8bit SRGB, but will select something else if this is not available
void SwapChain::chooseSwapSurfaceFormat() {
	const std::vector<VkSurfaceFormatKHR> availableFormats = swapChainSupport.formats;

	for (const auto& availableFormat : availableFormats) {
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB
			&& availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			surfaceFormat = availableFormat;
		}
	}
	//If we don't select the format we want above, this will select the first format available
	//Probably a better way to select "runner-up" choices than this though
	surfaceFormat = availableFormats[0];
}

//Similar function to chooseSwapSurfaceFormat but for "VSync" mode
void SwapChain::chooseSwapPresentMode() {
	const std::vector<VkPresentModeKHR> availablePresentModes = swapChainSupport.presentModes;

	for (const auto& availablePresentMode : availablePresentModes) {
		//Good mode for desktop. Uses a lot of energy, so not good for mobile
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
			presentMode = availablePresentMode;
		}
	}
	//This fallback present mode is guarenteed to be available on all devices that support presentation
	//Some tearing may occur (no VSYNC)
	presentMode = VK_PRESENT_MODE_FIFO_KHR;
}

//Chooses the resolution of the swap chain images
void SwapChain::chooseSwapExtent() {
	const VkSurfaceCapabilitiesKHR capabilities = swapChainSupport.capabilities;

	//If the extent is valid, and fits the window size already, use it.
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
		extent = capabilities.currentExtent;
	}
	//Otherwise, assign an extent from the window size manually
	else {
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);

		VkExtent2D actualExtent = {
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};

		actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

		extent = actualExtent;
	}
}

}