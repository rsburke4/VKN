#include "FrameBuffer.h"

namespace vkn {
	FrameBuffer::FrameBuffer(vkn::LogicalDevice* vknDevice,
		vkn::RenderPass* renderPass,
		uint32_t width,
		uint32_t height,
		uint32_t layers,
		VkImageView imageViewBind) {

		device = vknDevice;

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPass->getRenderPass();
		//This is fine as one for now, but we could bump it up later
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = &imageViewBind;
		framebufferInfo.width = width;
		framebufferInfo.height = height;
		framebufferInfo.layers = layers;

		if (vkCreateFramebuffer(device->getDevice(), &framebufferInfo, nullptr, &buffer)
			!= VK_SUCCESS) {
			throw std::runtime_error("failed to create framebuffer!");
		}
	}

	FrameBuffer::~FrameBuffer() {
		vkDestroyFramebuffer(device->getDevice(), buffer, nullptr);
	}

}