#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#ifndef __FRAME_BUFFER_H__
#define __FRAME_BUFFER_H__

#include "LogicalDevice.h"
#include "RenderPass.h"

namespace vkn {
	class FrameBuffer {
	public:
		FrameBuffer() {}
		FrameBuffer(vkn::LogicalDevice* vknDevice,
			vkn::RenderPass* renderPass,
			uint32_t width,
			uint32_t height,
			uint32_t layers,
			VkImageView imageViewBind);
		~FrameBuffer();

		VkFramebuffer getFrameBuffer() { return buffer; }

	private:
		vkn::LogicalDevice* device;
		VkFramebuffer buffer;

	};
}

#endif