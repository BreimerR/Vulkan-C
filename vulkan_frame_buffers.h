//
// Created by brymher on 29/01/25.
//

#ifndef VULKAN_FRAME_BUFFERS_H
#define VULKAN_FRAME_BUFFERS_H

#include <vulkan/vulkan.h>
#include "vulkan_window.h"

void createFrameBuffers(const VkDevice logicalDevice, VulkanWindow *vulkanWindow) {
    vulkanWindow->swapChainFrameBuffers.count = vulkanWindow->swapChainImagesViews.count;
    vulkanWindow->swapChainFrameBuffers.size = sizeof(VkFramebuffer) * vulkanWindow->swapChainFrameBuffers.count;
    vulkanWindow->swapChainFrameBuffers.items = malloc(vulkanWindow->swapChainFrameBuffers.size);

    for (int i = 0; i < vulkanWindow->swapChainFrameBuffers.count; i++) {
        const VkImageView attachments[] = {
            ((VkImageView *) vulkanWindow->swapChainImagesViews.items)[i]
        };

        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = vulkanWindow->renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = vulkanWindow->extent.width;
        framebufferInfo.height = vulkanWindow->extent.height;
        framebufferInfo.layers = 1;

        VkFramebuffer *frameBuffer = &((VkFramebuffer *) vulkanWindow->swapChainFrameBuffers.items)[i];
        if (vkCreateFramebuffer(logicalDevice, &framebufferInfo, nullptr, frameBuffer) != VK_SUCCESS) {
            printLn("Failed to create framebuffer! %d", i);
            exit(1);
        } else printLn("Successfully created frame buffer %d", i);
    }
}
#endif //VULKAN_FRAME_BUFFERS_H
