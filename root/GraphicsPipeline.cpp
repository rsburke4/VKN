#include "GraphicsPipeline.h"
#include "RenderPass.h"

#include <iostream>
#include <fstream>

namespace vkn {

	//Should convert all of these pointers to const pointers in the future
	GraphicsPipeline::GraphicsPipeline(LogicalDevice* logicalDevice, RenderPass* pass) {
		device = logicalDevice;
		renderPass = pass;
	}

	std::vector<char> GraphicsPipeline::readShaderFile(const std::string& filename) {
		//Open file, start reading at end to find out how big vector should be
		std::ifstream file(filename, std::ios::ate | std::ios::binary);
		if (!file.is_open()) {
			throw std::runtime_error("failed to open file");
		}
		size_t fileSize = (size_t)file.tellg();
		std::vector<char> buffer(fileSize);

		//Move back to the beginning to read forward
		file.seekg(0);
		file.read(buffer.data(), fileSize);
		file.close();

		return buffer;
	}

	VkShaderModule GraphicsPipeline::createShaderModule(const std::vector<char>& code) {
		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = code.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

		VkShaderModule shaderModule;
		if (vkCreateShaderModule(device->getDevice(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create valid shader module!");
		}
		return shaderModule;
	}

	void GraphicsPipeline::setVertexShader(std::string vertex) {
		std::vector<char> shaderCode = readShaderFile(vertex);
		vertexShader = createShaderModule(shaderCode);
	}


	void GraphicsPipeline::setTesselationShader(std::string tess) {
		std::vector<char> shaderCode = readShaderFile(tess);
		tesselationShader = createShaderModule(shaderCode);
	}

	void GraphicsPipeline::setFragmentShader(std::string frag) {
		std::vector<char> shaderCode = readShaderFile(frag);
		fragmentShader = createShaderModule(shaderCode);
	}

	void GraphicsPipeline::addBindingDescription(VkVertexInputBindingDescription bind) {
		bindingDescriptions.push_back(bind);
	}

	void GraphicsPipeline::addAttributeDescription(VkVertexInputAttributeDescription attr) {
		attributeDescriptions.push_back(attr);
	}

	//TODO Add stuff for tesselation shading
	void GraphicsPipeline::buildPipeline() {
		//Create Shader Stages
		VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = vertexShader;
		//Replace this with shader name in the future?
		vertShaderStageInfo.pName = "main";
		vertShaderStageInfo.pSpecializationInfo = nullptr; //Allows us to set compiletime const values in shader

		VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = fragmentShader;
		fragShaderStageInfo.pName = "main";
		fragShaderStageInfo.pSpecializationInfo = nullptr;

		//These now go into an array
		VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

		//Parameters that we will allow to be changed at runtime
		std::vector<VkDynamicState> dynamicStates = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
		};

		//Specifies the format of the vertex data input. For the demo, theres no format
		//This will change later.
		uint32_t bindCount = bindingDescriptions.size();
		uint32_t attrCount = attributeDescriptions.size();
		VkPipelineVertexInputStateCreateInfo vertexCreateInfo{};
		vertexCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexCreateInfo.vertexBindingDescriptionCount = bindCount;
		vertexCreateInfo.pVertexBindingDescriptions = bindCount > 0 ? bindingDescriptions.data() : nullptr;
		vertexCreateInfo.vertexAttributeDescriptionCount = attrCount;
		vertexCreateInfo.pVertexAttributeDescriptions = attrCount > 0 ? attributeDescriptions.data() : nullptr;

		//What kind of primitives to draw, given verties (Assembly stage)
		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST; //We want to draw disconnected triangles
		inputAssembly.primitiveRestartEnable = VK_FALSE; //We can breatup the lines and triangles from one another

		//Sets the scissor and viewport dynamically when you change the size of the window
		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
		dynamicState.pDynamicStates = dynamicStates.data();

		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.scissorCount = 1;
		//viewportState.pViewports = &viewport; //These lines are needed if not dynamic
		//viewportState.pScissors = &scissor;

		VkPipelineRasterizationStateCreateInfo rasterizer{};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE; //If true, items outside of the near and far planes will be smooshed to these planes
		rasterizer.rasterizerDiscardEnable = VK_FALSE; //If true, disables output to frame buffers. Why even have this?
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL; //Determines how fragments are generated. Other modes require GPU featues
		rasterizer.lineWidth = 1.0f; //How thick a line is in fragments. 1 is good. Thickers requires GPU featues;
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT; //Backface culling. Potentially we want to change this for certain materials?
		rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE; //Vertex ordering
		rasterizer.depthBiasEnable = VK_FALSE; //Alter depth values with a bias;
		rasterizer.depthBiasConstantFactor = 0.0f; //Optional
		rasterizer.depthBiasClamp = 0.0f; //Optionl
		rasterizer.depthBiasSlopeFactor = 0.0f; //Optional

		//Multisampling stage (antialiasing)
		//Disabled for now. Enabling requires a GPU feature
		VkPipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisampling.minSampleShading = 1.0f; //Optional
		multisampling.pSampleMask = nullptr; //Optional
		multisampling.alphaToCoverageEnable = VK_FALSE; //Optional;
		multisampling.alphaToOneEnable = VK_FALSE;

		//Color blending stage (When happens when one object is rendered on top of another)
		//This struct can be attatched to specific framebuffers
		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		//What channels should the blending apply to?
		colorBlendAttachment.colorWriteMask =
			VK_COLOR_COMPONENT_R_BIT |
			VK_COLOR_COMPONENT_G_BIT |
			VK_COLOR_COMPONENT_B_BIT |
			VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_TRUE;
		//These params determine how blending should occur
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
		colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

		/*For above, consider the following pseudocode :
		*
		if (blendEnable) {
			finalColor.rgb = (srcColorBlendFactor * newColor.rgb) <colorBlendOp> (dstColorBlendFactor * oldColor.rgb);
			finalColor.a = (srcAlphaBlendFactor * newColor.a) <alphaBlendOp> (dstAlphaBlendFactor * oldColor.a);
		} else {
			finalColor = newColor;
		}

			finalColor = finalColor & colorWriteMask;
		}

		Specifically, this basic alpha blending setup above has the form:

		finalColor.rgb = newAlpha * newColor + (1 - newAlpha) * oldColor;
		finalColor.a = newAlpha.a;

		*/

		//Color blending part 2
		//This apply globally
		VkPipelineColorBlendStateCreateInfo colorBlending{};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE; //Con combine fragments with bitwise logic
		colorBlending.logicOp = VK_LOGIC_OP_COPY; //Optional
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment; //A vector of all attatchments;
		colorBlending.blendConstants[0] = 0.0f; //Optional
		colorBlending.blendConstants[1] = 0.0f; //Optional
		colorBlending.blendConstants[2] = 0.0f; //Optional
		colorBlending.blendConstants[3] = 0.0f; //Optional

		//Any uniform variables (things that can be passed into a shader as a global
		// without the need for recreation of recompilation) need to be set up here
		//Even if we don't have any.
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0;
		pipelineLayoutInfo.pSetLayouts = nullptr;
		pipelineLayoutInfo.pushConstantRangeCount = 0; //Push constants are another way of passing data to a shader
		pipelineLayoutInfo.pPushConstantRanges = nullptr;

		if (vkCreatePipelineLayout(device->getDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create pipelineLayout!");
		}

		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2; //The number of custom stages
		pipelineInfo.pStages = shaderStages; //The custom stages themselves
		pipelineInfo.pVertexInputState = &vertexCreateInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pDepthStencilState = nullptr;
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDynamicState = &dynamicState;
		pipelineInfo.layout = pipelineLayout;

		pipelineInfo.renderPass = renderPass->getRenderPass();
		pipelineInfo.subpass = 0; //Index of the subpass that this pipeline will render to

		//This pipeline does not inherit from another pipeline
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
		pipelineInfo.basePipelineIndex = -1; //optional

		//This function is designed for caching to a file, and instancing multiple pipelines at the same time!
		//Seems like this could be usefuly elsewhere
		if (vkCreateGraphicsPipelines(device->getDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create graphics pipeline!");
		}

		//The existence of these lines implies that we never needed to hold the modules in a private class?
		// I'm going to keep these out of the code for now. At least until a better organizational structure is clear
		//vkDestroyShaderModule(device->getDevice(), fragmentShader, nullptr);
		//vkDestroyShaderModule(device->getDevice(), vertexShader, nullptr);

	}

	GraphicsPipeline::~GraphicsPipeline() {
		vkDestroyPipeline(device->getDevice(), graphicsPipeline, nullptr);
		vkDestroyPipelineLayout(device->getDevice(), pipelineLayout, nullptr);
	}
}