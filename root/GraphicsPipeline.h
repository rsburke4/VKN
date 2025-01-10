#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#ifndef __GRAPHICS_PIPELINE_H__
#define __GRAPHICS_PIPELINE_H__

#include <string>
#include <vector>
#include "LogicalDevice.h"
#include "RenderPass.h"

//This is where the meat of the application is.
//So much possibility here for cleanup and customization

namespace vkn{
	class GraphicsPipeline {
	public:
		//TODO Change this to a constructor that takes in custom shader objects later
		GraphicsPipeline() {}
		GraphicsPipeline(LogicalDevice *device, RenderPass *pass);
		~GraphicsPipeline();

		void setVertexShader(std::string vertex);
		void setTesselationShader(std::string tesselation);
		void setFragmentShader(std::string fragment);
		void addRenderPass(RenderPass* pass);


		void buildPipeline();
		VkPipeline getPipeline() { return graphicsPipeline; }


	private:
		std::vector<char> readShaderFile(const std::string& filename);
		VkShaderModule createShaderModule(const std::vector<char>& code);

		LogicalDevice *device;
		VkShaderModule vertexShader = VK_NULL_HANDLE;
		//TODO fallback shader for when no frag shader is present.
		VkShaderModule fragmentShader = VK_NULL_HANDLE;
		VkShaderModule tesselationShader = VK_NULL_HANDLE;
		VkShaderModule geometryShader;

		VkPipeline graphicsPipeline;
		VkPipelineLayout pipelineLayout;

		vkn::RenderPass *renderPass;
	};
}

#endif