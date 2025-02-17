//
// Created by brymher on 19/01/25.
//

#ifndef VULKAN_WINDOW_H
#define VULKAN_WINDOW_H
#include <vulkan/vulkan.h>
#include "vulkan_any.h"

typedef struct VulkanWindow {
    Any window;
    VkSurfaceKHR surface;
    VkSwapchainKHR swapChain;
    Uint32SizedMutableArray swapChainImages;
    Uint32SizedMutableArray swapChainImagesViews;
    VkFormat swapChainImageFormat;
    VkExtent2D extent;
    VkPipelineLayout pipelineLayout;
    VkRenderPass renderPass;
    VkPipeline graphicsPipeline;
    Uint32SizedMutableArray swapChainFrameBuffers; //VkFramebuffer
    VkCommandPool commandPool;
    Uint32SizedMutableArray commandBuffers; // VkCommandBuffer
    Uint32SizedMutableArray imageAvailableSemaphores; // VkSemaphore
    Uint32SizedMutableArray renderFinishedSemaphores; // VkSemaphore
    Uint32SizedMutableArray inFlightFences; // VkFence
    uint32_t MAX_FRAMES_IN_FLIGHT;
    uint32_t currentFrame;
    VkBuffer vertexBuffer;
    VkMemoryRequirements memRequirements;
    VkDeviceMemory vertexBufferMemory;
    bool resized;
} VulkanWindow;


#endif //VULKAN_WINDOW_H
