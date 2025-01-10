#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#ifndef __RENDER_PASS_H__
#define __RENDER_PASS_H__

#include "LogicalDevice.h"

namespace vkn {
	class RenderPass{
	public:
		RenderPass() {}
		RenderPass(LogicalDevice *logicalDevice, VkFormat format);
		~RenderPass();

		VkRenderPass getRenderPass() { return renderPass; }

	private:
		VkRenderPass renderPass;
		LogicalDevice* device;
	};
}

#endif