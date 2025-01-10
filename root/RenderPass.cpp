#include "RenderPass.h"

namespace vkn {

	RenderPass::RenderPass(LogicalDevice *logicalDevice, VkFormat format) {

		device = logicalDevice;

		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = format;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT; //No multisampling (For now)
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; //What happens to data already in the attachment
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE; //What happsn at the end of the renderpass
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE; //What happens to stencil data (we aren't using this)
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE; //What happens to stencil data post-renderpass
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; //What format flag the image has prior to renderpass
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; //What format flag the image should have post-renderpass (Chosen here for swap chain)

		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0; //Attachment index. Think-- Frag Shader: layout(location = 0) out vec4
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		//A subpass that utilizes one attachment
		//Every renderpass needs at least one subpass
		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS; //As opposed to compute shader pipeline or something
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef; //Could be a vector of all attachments needed

		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = 1;
		renderPassInfo.pAttachments = &colorAttachment;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		
		//This comes up later... Not sure where at the moment
		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		if (vkCreateRenderPass(device->getDevice(), &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create renderpass!");
		}

	}

	RenderPass::~RenderPass() {
		vkDestroyRenderPass(device->getDevice(), renderPass, nullptr);
	}
}