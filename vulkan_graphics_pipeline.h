//
// Created by brymher on 26/01/25.
//

#ifndef VULKAN_GRAPHICS_PIPELINE_H
#define VULKAN_GRAPHICS_PIPELINE_H

#include "constants.h"
#include <vulkan/vulkan.h>
#include <stdlib.h>
#include <string.h>
#include "io.h"
#include "glfw_app.h"
#include "array.h"
#include "vulkan_io.h"
#include "vulkan_vertex.h"

void createTriangleShaders(Uint32SizedMutableArray *vertexShader, Uint32SizedMutableArray *fragmentShader) {
    readFile("/opt/Projects/C/Vulkan/learning/resources/shaders/out/triangle.vert.spv", vertexShader);
    readFile("/opt/Projects/C/Vulkan/learning/resources/shaders/out/triangle.frag.spv", fragmentShader);
}

void createShaderModule(
    const VkDevice logicalDevice,
    VkShaderModule *shaderModule,
    const Uint32SizedMutableArray shader
) {
    const VkShaderModuleCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = shader.size,
        .pCode = (const uint32_t *) shader.items
    };

    if (vkCreateShaderModule(logicalDevice, &createInfo, nullptr, shaderModule) != VK_SUCCESS) {
        printLn("failed to create shader module!");
        exit(1);
    }
}


void populateVkViewPort(
    VkViewport *viewport,
    VkExtent2D swapChainExtent) {
    viewport->x = 0.0f;
    viewport->y = 0.0f;
    viewport->width = (float) swapChainExtent.width;
    viewport->height = (float) swapChainExtent.height;
    viewport->minDepth = 0.0f;
    viewport->maxDepth = 1.0f;
}

void populateVkAttachmentReference(VkAttachmentReference *colorAttachmentRef) {
    colorAttachmentRef->attachment = 0;
    colorAttachmentRef->layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
}

void populateVkSubpassDescription(VkSubpassDescription *subPassDescription, VkAttachmentReference *colorAttachmentRef) {
    subPassDescription->pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subPassDescription->colorAttachmentCount = 1;
    // This count comes from the shader directly. You might want to check on this when drawing many triangles
    subPassDescription->pColorAttachments = colorAttachmentRef;
}

void populateVkRenderPassCreateInfo(
    VkRenderPassCreateInfo *renderPassInfo,
    const VkAttachmentDescription *colorAttachment,
    const VkSubpassDescription *subPassDescription
) {
    renderPassInfo->sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo->attachmentCount = 1;
    renderPassInfo->pAttachments = colorAttachment;
    renderPassInfo->subpassCount = 1;
    renderPassInfo->pSubpasses = subPassDescription;


    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;

    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;

    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    renderPassInfo->dependencyCount = 1;
    renderPassInfo->pDependencies = &dependency;
}

void populateVkAttachmentDescription(VkAttachmentDescription *colorAttachment, const VulkanWindow *window) {
    colorAttachment->format = window->swapChainImageFormat;
    colorAttachment->samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment->loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment->storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment->stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment->stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment->initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment->finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
}

void createRenderPass(VulkanWindow *window, const VkDevice logicalDevice, VkRenderPass *renderPass) {
    VkAttachmentDescription colorAttachment = {};
    populateVkAttachmentDescription(&colorAttachment, window);

    VkAttachmentReference colorAttachmentRef = {};
    populateVkAttachmentReference(&colorAttachmentRef);

    VkSubpassDescription subPassDescription = {};
    populateVkSubpassDescription(&subPassDescription, &colorAttachmentRef);

    VkRenderPassCreateInfo renderPassInfo = {};
    populateVkRenderPassCreateInfo(&renderPassInfo, &colorAttachment, &subPassDescription);

    if (vkCreateRenderPass(logicalDevice, &renderPassInfo, nullptr, renderPass) != VK_SUCCESS) {
        printLn("Failed to create render pass!");
        exit(FAILED_TO_CREATE_RENDER_PASS);
    } else printLn("Created a render pass");
}

void createVulkanGraphicsPipeline(
    const VkDevice logicalDevice,
    VulkanWindow *vulkanWindow
) {
    Uint32SizedMutableArray vertexShader = {};
    Uint32SizedMutableArray fragmentShader = {};
    VkShaderModule vertShaderModule = {};
    VkShaderModule fragShaderModule = {};

    createTriangleShaders(&vertexShader, &fragmentShader);

    // TODO Memory might be getting lost here and I can't predict it.
    createShaderModule(logicalDevice, &vertShaderModule, vertexShader);
    createShaderModule(logicalDevice, &fragShaderModule, fragmentShader);


    VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    constexpr VkDynamicState dynamicStates[] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };
    VkPipelineDynamicStateCreateInfo dynamicState = {};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = 2;
    dynamicState.pDynamicStates = dynamicStates;

    VkVertexInputBindingDescription bindingDescription = {};
    getBindingDescription(&bindingDescription);

    Uint32SizedMutableArray attributeDescriptions = {
        .count = 2,
        .size = sizeof(VkVertexInputAttributeDescription) * 2,
        .items = malloc(sizeof(VkVertexInputAttributeDescription) * 2),
    };

    getAttributeDescriptions((VkVertexInputAttributeDescription *)attributeDescriptions.items);

    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription; // Optional
    vertexInputInfo.vertexAttributeDescriptionCount = attributeDescriptions.count; //
    vertexInputInfo.pVertexAttributeDescriptions = (VkVertexInputAttributeDescription *)attributeDescriptions.items; // Optional

    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;
    VkViewport viewport = {};

    VkRect2D scissor = {
        .offset = {0, 0},
        .extent = vulkanWindow->extent
    };

    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    populateVkViewPort(&viewport, vulkanWindow->extent);

    VkPipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f; // Optional
    rasterizer.depthBiasClamp = 0.0f; // Optional
    rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

    // Multisampling smoothing out edges
    VkPipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f; // Optional
    multisampling.pSampleMask = nullptr; // Optional
    multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
    multisampling.alphaToOneEnable = VK_FALSE; // Optional

    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT
                                          | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

    VkPipelineColorBlendStateCreateInfo colorBlending = {};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f; // Optional
    colorBlending.blendConstants[1] = 0.0f; // Optional
    colorBlending.blendConstants[2] = 0.0f; // Optional
    colorBlending.blendConstants[3] = 0.0f; // Optional

    VkGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;

    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = nullptr; // Optional
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = vulkanWindow->pipelineLayout;
    pipelineInfo.renderPass = vulkanWindow->renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
    pipelineInfo.basePipelineIndex = -1; // Optional

    if (vkCreateGraphicsPipelines(logicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr,
                                  &(vulkanWindow->graphicsPipeline)) != VK_SUCCESS) {
        printLn("Failed to create graphics pipeline!");
        exit(1);
    } else printLn("Created the graphis pipeline sucessfully");

    // not sure when to clean this part up
    vkDestroyShaderModule(logicalDevice, fragShaderModule, nullptr);
    vkDestroyShaderModule(logicalDevice, vertShaderModule, nullptr);
}

void initVulkanGraphicsPipeline(
    const VkDevice logicalDevice,
    VulkanWindow *vulkanWindow
) {
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0; // Optional
    pipelineLayoutInfo.pSetLayouts = nullptr; // Optional
    pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
    pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

    if (vkCreatePipelineLayout(logicalDevice, &pipelineLayoutInfo, nullptr, &(vulkanWindow->pipelineLayout)) !=
        VK_SUCCESS) {
        printLn("Failed to create Vulkan Pipeline Layout");
        exit(1);
    }

    createRenderPass(vulkanWindow, logicalDevice, &(vulkanWindow->renderPass));

    createVulkanGraphicsPipeline(logicalDevice, vulkanWindow);

    printLn("Crated Vulkan Pipeline Layout fine");
}
#endif //VULKAN_GRAPHICS_PIPELINE_H
