#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <iostream>
#include <array>

#include <stdexcept>
#include <cstdlib>
#include <vector>
#include <map>
#include <optional>
#include <set>
#include <cstdint> // Necessary for uint32_t
#include <limits> // Necessary for std::numeric_limits
#include <algorithm> // Necessary for std::clamp
#include <fstream>

//Custom headers for encapsulation
#include "VulkanInstance.h"
#include "PhysicalDevice.h"
#include "LogicalDevice.h"
#include "SwapChain.h"
#include "RenderPass.h"
#include "GraphicsPipeline.h"
#include "FrameBuffer.h"

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;
const int32_t MAX_FRAMES_IN_FLIGHT = 2;

//Required validation layers
const std::vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};

//Required extensions
const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

//Enable validation layers for debug mode
#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif


struct Vertex {
	glm::vec2 pos;
	glm::vec3 color;

	//Defines the rate to sample data. How big is each chunk of data etc .
	static VkVertexInputBindingDescription getBindingDescription() {
		VkVertexInputBindingDescription bindingDescription{};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescription;
	}

	//Defines how to extract data from bindings. We need one for each attr.
	//Two here, in the case where we care about position and color
	static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions() {
		std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};
		//Position description
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, pos);

		//Color description
		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, color);

		return attributeDescriptions;

	}
};

const std::vector<Vertex> vertices = {
	{{0.0f, -0.5}, {1.0f, 0.0f, 0.0f}},
	{{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
	{{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
};


//Gives a score to physical devices to determine which is best
//Based on what we want it to do
//In this case, high texture resolution, and dedication level
int rateDeviceSuitability(VkPhysicalDevice device) {
	VkPhysicalDeviceProperties deviceProperties;
	VkPhysicalDeviceFeatures deviceFeatures;
	//Gets physical properties of device
	vkGetPhysicalDeviceProperties(device, &deviceProperties);
	//Gets feature capabilities of device
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

	int score = 0;

	if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
		score += 1000;
	}
	score += deviceProperties.limits.maxImageDimension2D;

	if (!deviceFeatures.geometryShader) {
		return 0;
	}

	return score;
}

class HelloTriangleApplication {
public:

	void run() {
		initWindow();
		initVulkan();
		mainLoop();
		cleanup();
	}

private:

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData) {

		std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

		return false;
	}

	void initWindow() {
		//Make a GLFW Window, and make it agnostic to api (OpenGL is default context.
		// We don't want this.)
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		//Lets make windows not-resizable for now

		//Create actual window
		window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
		glfwSetWindowUserPointer(window, this);
		glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
	}

	static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
		auto app = reinterpret_cast<HelloTriangleApplication*>(glfwGetWindowUserPointer(window));
		app->framebufferResized = true;
	}

	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
		createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = debugCallback; //The function we made above
		createInfo.pUserData = nullptr; //Optional
	}

	void cleanupSwapChain() {
		for (size_t i = 0; i < swapChainFramebuffers.size(); i++) {
			delete(swapChainFramebuffers[i]);
		}

		for (size_t i = 0; i < swapChainImageViews.size(); i++) {
			vkDestroyImageView(vknDevice->getDevice(), swapChainImageViews[i], nullptr);
		}

		delete(vknSwapChain);
	}

	void recreateSwapChain() {
		//Special Minimizing Case
		int width = 0, height = 0;
		glfwGetFramebufferSize(window, &width, &height);
		while (width == 0 || height == 0) {
			glfwGetFramebufferSize(window, &width, &height);
			glfwWaitEvents();
		}

		vkDeviceWaitIdle(vknDevice->getDevice());

		cleanupSwapChain();

		//TODO Add reference to old swap chain. Differentiate between new and old swap chains
		vknSwapChain = new vkn::SwapChain(vknPhysicalDevice, vknDevice, surface, window);
		createImageViews();
		createFramebuffers();

	}

	void createSurface() {
		//GLFW makes this simple.
		if (glfwCreateWindowSurface(vknInstance->getInstance(), window, nullptr, &surface) != VK_SUCCESS) {
			throw std::runtime_error("failed to create window surface!");
		}
	}

	void createImageViews() {
		swapChainImageViews.resize(vknSwapChain->getImages().size());
		for (size_t i = 0; i < vknSwapChain->getImages().size(); i++) {
			VkImageViewCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.image = vknSwapChain->getImages()[i];
			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			createInfo.format = vknSwapChain->getFormat().format;

			//We can "swizzle" channels around here
			createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

			//What the image is used for. In this case, color target with no mipmapping and a single layer
			createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 1;

			if (vkCreateImageView(vknDevice->getDevice(), &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to create image views!");
			}
		}
	}

	void createFramebuffers() {
		swapChainFramebuffers.resize(swapChainImageViews.size());
		for (size_t i = 0; i < swapChainImageViews.size(); i++) {
			VkImageView attachments[] = {
				swapChainImageViews[i]
			};

			swapChainFramebuffers[i] = new vkn::FrameBuffer(vknDevice,
				vknRenderPass,
				vknSwapChain->getExtent().width,
				vknSwapChain->getExtent().height,
				1,
				swapChainImageViews[i]);

		}
	}

	//Command pools manage the memory used in command buffers.
	void createCommandPool() {
		vkn::QueueFamilyIndices queueFamilyIndices = vknPhysicalDevice->findQueueFamilies(surface);

		//This is a command pool for the graphics queue. Should there not also be
		//One for the presentation queue in case they are different? We'll see....
		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

		if (vkCreateCommandPool(vknDevice->getDevice(), &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
			throw std::runtime_error("failed to create command pool!");
		}
	}

	void createCommandBuffers() {
		commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = commandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

		if (vkAllocateCommandBuffers(vknDevice->getDevice(), &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate command buffers!");
		}
	}

	void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = 0;
		beginInfo.pInheritanceInfo = nullptr;

		if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error("failed to begin recording command buffer!");
		}

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		//renderPassInfo.renderPass = renderPass;
		renderPassInfo.renderPass = vknRenderPass->getRenderPass();
		renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex]->getFrameBuffer();
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = vknSwapChain->getExtent();

		VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearColor;

		//Time to record render pass into command buffer
		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vknGraphicsPipeline->getPipeline());
		//We call this because we are setting viewport and scissor dynamically
				//Viewport that will be used
		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)vknSwapChain->getExtent().width;
		viewport.height = (float)vknSwapChain->getExtent().height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

		//Scissor area
		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = vknSwapChain->getExtent();
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

		VkBuffer vertexBuffers[] = { vertexBuffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
		vkCmdDraw(commandBuffer, static_cast<uint32_t>(vertices.size()), 1, 0, 0);

		vkCmdEndRenderPass(commandBuffer);
		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to record command buffer!");
		}
	}

	//Semafores alert to when gpu work is done.
	//Fences block cpu completely until signaled.
	void createSyncObjects() {
		imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			if (vkCreateSemaphore(vknDevice->getDevice(), &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS
				|| vkCreateSemaphore(vknDevice->getDevice(), &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS
				|| vkCreateFence(vknDevice->getDevice(), &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to create semaphore or fence!");
			}
		}
	}

	void drawFrame() {
		vkWaitForFences(vknDevice->getDevice(), 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

		uint32_t imageIndex;
		VkResult result = vkAcquireNextImageKHR(vknDevice->getDevice(), vknSwapChain->getSwapChain(), UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex); //Get next swapchain image

		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			recreateSwapChain();
			return;
		}
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("failed to aquire swap chain image!");
		}
		//Only reset the fences if we are submitting work
		vkResetFences(vknDevice->getDevice(), 1, &inFlightFences[currentFrame]);

		//Record command buffer
		vkResetCommandBuffer(commandBuffers[currentFrame], 0); //Reset command buffer
		recordCommandBuffer(commandBuffers[currentFrame], imageIndex);

		//Queue submission and syncronization
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		//Sync infor for rendering
		VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame]};
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores; //This should have the same index
		submitInfo.pWaitDstStageMask = waitStages; // as this
		//Which command buffers to submit for execution
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffers[currentFrame];

		VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
			throw std::runtime_error("failed to submit draw command buffer!");
		}

		//Sync info for presentation
		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		VkSwapchainKHR swapChains[] = { vknSwapChain->getSwapChain() };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &imageIndex;
		presentInfo.pResults = nullptr; //Good for multiple swap chains

		//Throws results on screen
		result = vkQueuePresentKHR(presentQueue, &presentInfo);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
			framebufferResized = false;
			recreateSwapChain();
		}
		else if (result != VK_SUCCESS) {
			throw std::runtime_error("failed to present swap chain image!");
		}

		currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
	}

	void initVulkan() {
		//An instance is the connection between your app and Vulkan API.
		//This "wakes up" the Api.
		vknInstance = new vkn::VulkanInstance(enableValidationLayers);
		createSurface();

		//Device Selection
		//pickPhysicalDevice();
		vknPhysicalDevice = new vkn::PhysicalDevice(vknInstance, surface);

		//Logical Device Creation
		//could be cleaned up with encapsulation of VkQueue object
		vknDevice = new vkn::LogicalDevice(vknPhysicalDevice, surface, enableValidationLayers, validationLayers);
		vkn::QueueFamilyIndices indices = vknPhysicalDevice->findQueueFamilies(surface);
		vknDevice->getDeviceQueue(indices.graphicsFamily.value(), 0, &graphicsQueue);
		vknDevice->getDeviceQueue(indices.presentFamily.value(), 0, &presentQueue);

		vknSwapChain = new vkn::SwapChain(vknPhysicalDevice, vknDevice, surface, window);

		createImageViews();
		//createRenderPass();
		vknRenderPass = new vkn::RenderPass(vknDevice, vknSwapChain->getFormat().format);

		//createGraphicsPipeline();
		vknGraphicsPipeline = new vkn::GraphicsPipeline(vknDevice, vknRenderPass);
		vknGraphicsPipeline->setVertexShader("root/shaders/compiled/vertS.spv");
		vknGraphicsPipeline->setFragmentShader("root/shaders/compiled/frag.spv");
		auto bindingDescription = Vertex::getBindingDescription();
		auto attributeDescriptions = Vertex::getAttributeDescriptions();
		vknGraphicsPipeline->addBindingDescription(bindingDescription);
		for (size_t i = 0; i < attributeDescriptions.size(); i++) {
			vknGraphicsPipeline->addAttributeDescription(attributeDescriptions[i]);
		}
		vknGraphicsPipeline->buildPipeline();


		createFramebuffers();
		createCommandPool();
		createVertexBuffer();
		createCommandBuffers();
		createSyncObjects();
	}

	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(vknPhysicalDevice->getPhysicalDevice(), &memProperties);

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
			if (typeFilter & (1 << i) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
				return i;
			}
		}

		throw std::runtime_error("failed to find suitable memory type!");
	}

	//Current implimentation is not very general.
	//This could be much more generalized
	void createVertexBuffer() {
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = sizeof(vertices[0]) * vertices.size();
		bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateBuffer(vknDevice->getDevice(), &bufferInfo, nullptr, &vertexBuffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to create vertex buffer!");
		}

		//Gather memory requirements and allocate memory
		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(vknDevice->getDevice(), vertexBuffer, &memRequirements);
	
		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		if (vkAllocateMemory(vknDevice->getDevice(), &allocInfo, nullptr, &vertexBufferMemory) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate vertex buffer memory!");
		}
		vkBindBufferMemory(vknDevice->getDevice(), vertexBuffer, vertexBufferMemory, 0);

		void* data;
		vkMapMemory(vknDevice->getDevice(), vertexBufferMemory, 0, bufferInfo.size, 0, &data);
		memcpy(data, vertices.data(), (size_t) bufferInfo.size);
		vkUnmapMemory(vknDevice->getDevice(), vertexBufferMemory);
	}

	void mainLoop() {
		//Main while loop as long as window is open.
		while (!glfwWindowShouldClose(window)) {
			glfwPollEvents();
			drawFrame();
		}
		vkDeviceWaitIdle(vknDevice->getDevice());
	}

	//Basically init, but backwards
	void cleanup() {
		cleanupSwapChain();
		vkDestroyBuffer(vknDevice->getDevice(), vertexBuffer, nullptr);
		vkFreeMemory(vknDevice->getDevice(), vertexBufferMemory, nullptr);


		delete(vknGraphicsPipeline);
		delete(vknRenderPass);

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			vkDestroySemaphore(vknDevice->getDevice(), imageAvailableSemaphores[i], nullptr);
			vkDestroySemaphore(vknDevice->getDevice(), renderFinishedSemaphores[i], nullptr);
			vkDestroyFence(vknDevice->getDevice(), inFlightFences[i], nullptr);
		}

		vkDestroyCommandPool(vknDevice->getDevice(), commandPool, nullptr);

		//This deletes logical devices. Even though the physical is the param
		//Physical devices get killed when the instance is destroyed
		delete(vknDevice);

		vkDestroySurfaceKHR(vknInstance->getInstance(), surface, nullptr);
		delete(vknInstance);

		glfwDestroyWindow(window);

		glfwTerminate();
	}

	//Custom Classes for Encapsulation
	vkn::VulkanInstance* vknInstance;

	//Window where things are rendered
	GLFWwindow* window;
	//Surface (Like a logical link to the "physical" window)
	VkSurfaceKHR surface;
	//Instance of Vulkan we are using
	VkInstance instance;
	//Debug Messenger Extension we use for callbacks
	VkDebugUtilsMessengerEXT debugMessenger;
	//The physical device we will be using
	
	//Destroyed when instance is destroyed
	vkn::PhysicalDevice* vknPhysicalDevice;

	//Logical Device
	vkn::LogicalDevice* vknDevice;

	//The graphics capable queue we will be using
	VkQueue graphicsQueue;
	//The presentation queue we will be using
	VkQueue presentQueue;

	//Swap Chain for rendering images
	//Contains images, formats etc
	vkn::SwapChain *vknSwapChain;

	std::vector<VkImageView> swapChainImageViews;

	//Holds the pipeline layout
	vkn::RenderPass *vknRenderPass;
	vkn::GraphicsPipeline *vknGraphicsPipeline;

	//Framebuffers
	std::vector<vkn::FrameBuffer*> swapChainFramebuffers;

	//Command Buffers
	VkCommandPool commandPool;
	std::vector<VkCommandBuffer> commandBuffers;

	//Syncronization objects
	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderFinishedSemaphores;
	std::vector<VkFence> inFlightFences;
	uint32_t currentFrame = 0;

	//Manually detect when window is resized
	bool framebufferResized = false;

	//Vertex buffer data
	VkBuffer vertexBuffer;
	VkDeviceMemory vertexBufferMemory;
};

int main() {
	HelloTriangleApplication app;

	try {
		app.run();
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}