//
// Created by brymher on 29/01/25.
//

#ifndef VULKAN_COMMAND_BUFFERS_H
#define VULKAN_COMMAND_BUFFERS_H

#include <vulkan/vulkan.h>
#include "vulkan_window.h"
#include "vulkan_io.h"
#include <stdlib.h>
#include "constants.h"
#include "vulkan_vertex.h"

void createCommandPool(const VkDevice logicalDevice, VkCommandPool *commandPool, const uint32_t queueFamilyIndex) {
    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndex;

    if (vkCreateCommandPool(logicalDevice, &poolInfo, nullptr, commandPool) != VK_SUCCESS) {
        printLn("failed to create command pool");
        exit(1);
    } else printLn("created command pool");
}

void createCommandBuffers(
    const VkDevice logicalDevice,
    VulkanWindow *vulkanWindow
) {
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = vulkanWindow->commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = MAX_FRAMES_IN_FLIGHT;

    vulkanWindow->commandBuffers.count = MAX_FRAMES_IN_FLIGHT;
    vulkanWindow->commandBuffers.size = MAX_FRAMES_IN_FLIGHT * sizeof(VkCommandBuffer);
    vulkanWindow->commandBuffers.items = malloc(vulkanWindow->commandBuffers.size);

    if (vkAllocateCommandBuffers(logicalDevice, &allocInfo, (VkCommandBuffer *) vulkanWindow->commandBuffers.items) !=
        VK_SUCCESS) {
        printLn("failed to allocate command buffers!");
        exit(2);
    } else printLn("command buffers created");
}

void vulkanCmdSetViewport(const VulkanWindow *window) {
    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float) window->extent.width;
    viewport.height = (float) window->extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(
        ((VkCommandBuffer *) window->commandBuffers.items)[window->currentFrame],
        0,
        1,
        &viewport
    );
}

void vulkanCmdSetScissor(const VulkanWindow *window) {
    VkRect2D scissor = {};
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    scissor.extent = window->extent;
    vkCmdSetScissor(
        ((VkCommandBuffer *) window->commandBuffers.items)[window->currentFrame]
        , 0,
        1,
        &scissor
    );
}

void vulkanSubmitRenderPass(const VulkanWindow *window, const uint32_t imageIndex) {
    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = window->renderPass;
    renderPassInfo.framebuffer = ((VkFramebuffer *) window->swapChainFrameBuffers.items)[imageIndex];

    const VkOffset2D offset = {0, 0};
    renderPassInfo.renderArea.offset = offset;
    renderPassInfo.renderArea.extent = window->extent;

    const VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 0.0f}}};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(
        ((VkCommandBuffer *) window->commandBuffers.items)[window->currentFrame],
        &renderPassInfo,
        VK_SUBPASS_CONTENTS_INLINE
    );

    printLn("Submitted render pass");
}

void beginRenderPass(const VulkanWindow *window, const uint32_t imageIndex) {
    vulkanSubmitRenderPass(window, imageIndex);
    const VkCommandBuffer commandBuffer = ((VkCommandBuffer *) window->commandBuffers.items)[window->currentFrame];
    vkCmdBindPipeline(
        commandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        window->graphicsPipeline
    );

    const VkBuffer vertexBuffers[] = {window->vertexBuffer};
    constexpr VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(commandBuffer, window->indexBuffer, 0, VK_INDEX_TYPE_UINT32); // this section needs to be the same as the index buffer index types

    vulkanCmdSetScissor(window);
    vulkanCmdSetViewport(window);

    vkCmdDrawIndexed(commandBuffer, 6, 1, 0, 0, 0);

    vkCmdEndRenderPass(commandBuffer);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        printLn("failed to record command buffer!");
        exit(VULKAN_FAILED_TO_END_COMMAND_BUFFER);
    } else printLn("Ended command buffer %d", window->currentFrame);
}

void recordCommandBuffer(const VulkanWindow *window, const uint32_t imageIndex) {
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0; // Optional
    beginInfo.pInheritanceInfo = nullptr; // Optional

    printLn("Recording command buffer %d", window->currentFrame);
    const VkResult res = vkBeginCommandBuffer(
        ((VkCommandBuffer *) window->commandBuffers.items)[window->currentFrame],
        &beginInfo
    );

    if (res != VK_SUCCESS) {
        printLn("Failed to begin recording command buffer!");
        exit(3);
    }

    beginRenderPass(window, imageIndex);
}

void createSyncObjects(
    const VkDevice logicalDevice,
    VulkanWindow *window
) {
    printLn("createSyncObjects");

    window->inFlightFences.count = MAX_FRAMES_IN_FLIGHT;
    window->inFlightFences.size = sizeof(VkFence) * MAX_FRAMES_IN_FLIGHT;
    window->inFlightFences.items = malloc(window->inFlightFences.size);
    printLn("Sized inFlightSemaphores");

    size_t semaphoresSize = sizeof(VkSemaphore) * MAX_FRAMES_IN_FLIGHT;

    window->imageAvailableSemaphores.count = MAX_FRAMES_IN_FLIGHT;
    window->imageAvailableSemaphores.size = semaphoresSize;
    window->imageAvailableSemaphores.items = malloc(semaphoresSize);
    printLn("Sized imageAvailableSemaphores");

    window->renderFinishedSemaphores.count = MAX_FRAMES_IN_FLIGHT;
    window->renderFinishedSemaphores.size = semaphoresSize;
    window->renderFinishedSemaphores.items = malloc(semaphoresSize);
    printLn("Sized renderFinishedSemaphores");

    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        printLn("Assigning sync objects %d", i);

        const VkResult availableSemaphoreResult = vkCreateSemaphore(
            logicalDevice,
            &semaphoreInfo,
            nullptr,
            &((VkSemaphore *) window->imageAvailableSemaphores.items)[i]
        );

        const VkResult renderFinishedSemaphoreResult = vkCreateSemaphore(
            logicalDevice,
            &semaphoreInfo,
            nullptr,
            &((VkSemaphore *) window->renderFinishedSemaphores.items)[i]
        );

        const VkResult inFlightFenceResult = vkCreateFence(
            logicalDevice,
            &fenceInfo,
            nullptr,
            &((VkFence *) window->inFlightFences.items)[i]
        );

        if (
            availableSemaphoreResult != VK_SUCCESS ||
            renderFinishedSemaphoreResult != VK_SUCCESS ||
            inFlightFenceResult != VK_SUCCESS
        ) {
            printLn("Failed to create semaphores!");
            exit(1);
        } else printLn("Created initial sync objects");
    }
}

void drawFrame(
    GLFWApp *app,
    VulkanWindow *window,
    const VkQueue presentQueue,
    const VkQueue graphicsQueue
) {
    // const VkDevice logicalDevice,const VkFence *inFlightFence
    const VkFence *vulkanFence = &((VkFence *) window->inFlightFences.items)[window->currentFrame];
    vkWaitForFences(app->logicalDevice, 1, vulkanFence, VK_TRUE, UINT64_MAX);
    vkResetFences(app->logicalDevice, 1, vulkanFence);

    uint32_t imageIndex;
    VkResult acquireNextImageResult = vkAcquireNextImageKHR(
        app->logicalDevice,
        window->swapChain,
        UINT64_MAX,
        ((VkSemaphore *) window->imageAvailableSemaphores.items)[window->currentFrame],
        VK_NULL_HANDLE,
        &imageIndex
    );
    if (acquireNextImageResult == VK_ERROR_OUT_OF_DATE_KHR || acquireNextImageResult == VK_SUBOPTIMAL_KHR || window->
        resized) {
        window->resized = false;
        printLn("Failed to acquire swap chain image!");
        recreateSwapChain(app);
        return;
    } else if (acquireNextImageResult != VK_SUCCESS && acquireNextImageResult != VK_SUBOPTIMAL_KHR) {
        printLn("failed to acquire swap chain image!");
        exit(1);
    }


    const VkCommandBuffer commandBuffer = ((VkCommandBuffer *) window->commandBuffers.items)[window->currentFrame];
    vkResetCommandBuffer(commandBuffer, 0);
    recordCommandBuffer(window, imageIndex);

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    const VkSemaphore waitSemaphores[] = {
        ((VkSemaphore *) window->imageAvailableSemaphores.items)[window->currentFrame]
    };
    constexpr VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &((VkCommandBuffer *) window->commandBuffers.items)[window->currentFrame];

    const VkSemaphore signalSemaphores[] = {
        ((VkSemaphore *) window->renderFinishedSemaphores.items)[window->currentFrame]
    };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    printLn("Presentation queue found %d, %d", sizeof(graphicsQueue), sizeof(int));

    if (graphicsQueue == VK_NULL_HANDLE) {
        printLn("Present queue isn't available");
        exit(1);
    } else printLn("Presentation queue is created");

    const VkFence vkFence = ((VkFence *) window->inFlightFences.items)[window->currentFrame];

    if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, vkFence) != VK_SUCCESS) {
        printLn("failed to submit draw command buffer!");
        exit(1);
    } else printLn("Created inflight fence from graphics queue");

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    const VkSwapchainKHR swapChains[] = {window->swapChain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;

    presentInfo.pResults = nullptr; // Optional

    vkQueuePresentKHR(presentQueue, &presentInfo);

    window->currentFrame = (window->currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void createIndexBuffer(
    VulkanWindow *window,
    BufferVertices buffserVertices,
    VkQueue graphicsQueue,
    VkDevice logicalDevice,
    VkPhysicalDevice physicalDevice
) {
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    createBuffer(
        &stagingBuffer,
        buffserVertices.indices.size,
        &stagingBufferMemory,
        &window->memRequirements,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        physicalDevice,
        logicalDevice
    );

    Any data;
    vkMapMemory(logicalDevice, stagingBufferMemory, 0, buffserVertices.indices.size, 0, &data);
    memcpy(data, (uint32_t *) buffserVertices.indices.items, buffserVertices.indices.size);
    vkUnmapMemory(logicalDevice, stagingBufferMemory);

    createBuffer(
        &window->indexBuffer,
        buffserVertices.indices.size,
        &window->indexBufferMemory,
        &window->memRequirements,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        physicalDevice,
        logicalDevice
    );

    copyBuffer(
        stagingBuffer,
        window->indexBuffer,
        buffserVertices.indices.size,
        window->commandPool,
        logicalDevice,
        graphicsQueue
    );

    vkDestroyBuffer(logicalDevice, stagingBuffer, nullptr);
    vkFreeMemory(logicalDevice, stagingBufferMemory, nullptr);
}

void initCommandBuffers(
    const VkPhysicalDevice physicalDevice,
    const VkDevice logicalDevice,
    VulkanWindow *window,
    const uint32_t queueFamilyIndex,
    VkQueue graphicsQueue
) {
    createCommandPool(logicalDevice, &(window->commandPool), queueFamilyIndex);

    const BufferVertices bufferVertices = {
        .vertices = {
            .count = 4,
            .size = sizeof(Vertex) * 4,
            .items = (Any *) (Vertex []){
                {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
                {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
                {{0.5f, 0.5f}, {0.0f, 0.0f, 0.0f}},
                {{-0.5f, 0.5f}, {1.0f, 1.0f, 0.0f}}
            }
        },
        .indices = {
            .count = 6,
            .size = sizeof(uint32_t) * 6,
            .items = (Any *) (uint32_t []){
                0, 1, 2, 2, 3, 0
            }
        }

    };

    createVertexBuffer(physicalDevice, logicalDevice, bufferVertices, window, graphicsQueue);
    createIndexBuffer(window, bufferVertices, graphicsQueue, logicalDevice, physicalDevice);
    createCommandBuffers(logicalDevice, window);
    createSyncObjects(logicalDevice, window);

    printLn("Done creating Sync objects");
}
#endif //VULKAN_COMMAND_BUFFERS_H`
