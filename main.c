#include <stdio.h>

#ifndef GLFW_INCLUDE_VULKAN
#define GLFW_INCLUDE_VULKAN

#include "glfw.h"
#include "array.h"
#include "vulkan.h"
#include "vulkan_window.h"
#include "vulkan_callbacks.h"


void cleanup(GLFWApp app);

void startGLFWWindowLoop( GLFWApp *app) {
    while (!glfwWindowShouldClose(getCurrentGLFWAppWindow(*app))) {
        glfwPollEvents();

        drawFrame(
            app,
            getCurrentVulkanWindow(*app),
            app->presentQueue,
            app->graphicsQueue
        );
    }

    vkDeviceWaitIdle(app->logicalDevice);
}


int main(void) {
    GLFWApp app = {
        .name = "LEARNING APPLICATION",
        .windows = createUint32SizedMutableArray(),
        .physicalDevices = createUint32SizedMutableArray(),
        .currentPhysicalDevice = -1,
        .queueFamilyIndex = -1,
        .presentFamilyIndex = -1,
        .graphicsQueue = VK_NULL_HANDLE,
        .presentQueue = VK_NULL_HANDLE
    };
    glfwInit();
    disableOpenGL();
    //disableResize();

    const Uint32SizedMutableArray enabledExtensionsArray = {
        .size = sizeof(char) * 18,
        .count = 1,
        .items = (Any*) (
            (char *[]){
                VK_EXT_DEBUG_UTILS_EXTENSION_NAME
            }
        )
    };

    const Uint32SizedMutableArray requestLayerExtensions = {
        .size = sizeof(char) * 27,
        .count = 2,
        .items = (Any*) (
            (char *[]){
                "VK_LAYER_KHRONOS_validation",
                //"VK_LAYER_MESA_device_select"//,
                "VK_LAYER_INTEL_nullhw"
                // "VK_LAYER_MESA_overlay",
                // "VK_LAYER_NV_optimus"
            }
        )
    };

    initVulkan(&app, enabledExtensionsArray, requestLayerExtensions);
    prepareVulkanApp(&app);
    startGLFWWindowLoop(&app);
    cleanup(app);
    return 0;
}


void cleanup(const GLFWApp app) {
    printLn("Cleaning up");
    VkAllocationCallbacks callbacks = {
        .pUserData = "Smile More",
        .pfnAllocation = pfnvkAllocationFunction,
        .pfnReallocation = pfnvkReallocationFunction,
        .pfnFree = pfnvkFreeFunction,
        .pfnInternalAllocation = fn_vkInternalAllocationNotification,
        .pfnInternalFree = pfn_vkInternalFreeNotification
    };

    for (uint32_t i = 0; i < app.windows->count; i++) {
        VulkanWindow *vulkanWindow = getVulkanWindowAt(i, app);

        cleanUpSwapChain(app.logicalDevice, vulkanWindow);

        printLn("Second level cleanup");


        callbacks.pUserData = "vkDestroyRenderPass";
        vkDestroyRenderPass(app.logicalDevice, vulkanWindow->renderPass, &callbacks);
        callbacks.pUserData = "vkDestroyPipeline";
        vkDestroyPipeline(app.logicalDevice, vulkanWindow->graphicsPipeline, nullptr);
        callbacks.pUserData = "vkDestroyPipelineLayout";
        vkDestroyPipelineLayout(app.logicalDevice, vulkanWindow->pipelineLayout, &callbacks);

        vkDestroyCommandPool(app.logicalDevice, vulkanWindow->commandPool, nullptr);

        for (size_t k = 0; k < MAX_FRAMES_IN_FLIGHT; k++) {
            vkDestroySemaphore(
                app.logicalDevice,
                ((VkSemaphore *) vulkanWindow->imageAvailableSemaphores.items)[k],
                nullptr
            );
            vkDestroySemaphore(
                app.logicalDevice,
                ((VkSemaphore *) vulkanWindow->renderFinishedSemaphores.items)[k],
                nullptr
            );
            vkDestroyFence(
                app.logicalDevice,
                ((VkFence *) vulkanWindow->inFlightFences.items)[k],
                nullptr
            );
        }


        vkDestroyBuffer(app.logicalDevice, vulkanWindow->indexBuffer, nullptr);
        vkFreeMemory(app.logicalDevice,  vulkanWindow->indexBufferMemory, nullptr);

        vkFreeMemory(app.logicalDevice, vulkanWindow->vertexBufferMemory, nullptr);
        vkDestroyBuffer(app.logicalDevice, vulkanWindow->vertexBuffer, nullptr);


        glfwDestroyWindow(vulkanWindow->window);
    }

    cleanUpVulkan(app);
}

#endif
