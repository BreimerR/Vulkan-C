//
// Created by brymher on 18/12/24.
//

#ifndef LEARNING_GLFW_APP_H
#define LEARNING_GLFW_APP_H

#include <GLFW/glfw3.h>
#include "array.h"
#include "vulkan_window.h"

typedef struct GLFWApp {
    const char *name;
    Uint32SizedMutableArray *windows;
    Uint32SizedMutableArray *physicalDevices;
    int currentPhysicalDevice;
    VkDevice logicalDevice; // This needs to be logical device in order to support multiple GPU support
    uint32_t currentWindow;
    VkQueue graphicsQueue;
    uint32_t rootWindow;
    VkInstance *vkInstance;
    VkDebugUtilsMessengerEXT *debugMessenger;
    uint32_t queueFamilyIndex;
    uint32_t presentFamilyIndex;
    VkQueue presentQueue;
} GLFWApp;


VulkanWindow *getCurrentVulkanWindow(const GLFWApp app) {
    return ((VulkanWindow **) app.windows->items)[app.currentWindow];
}

GLFWwindow *getCurrentGLFWAppWindow(const GLFWApp app) {
    return getCurrentVulkanWindow(app)->window;
}

VkSwapchainKHR *getCurrentVulkanSwapChain(const GLFWApp app) {
    return &getCurrentVulkanWindow(app)->swapChain;
}

VulkanWindow *getVulkanWindowAt(const uint32_t index, const GLFWApp app) {
    return ((VulkanWindow **) app.windows->items)[index];
}

GLFWwindow *getRootGLFWAppWindow(const GLFWApp app) {
    return getVulkanWindowAt(app.rootWindow, app)->window;
}

GLFWwindow *getGLFWWindowAt(const uint32_t index, const GLFWApp app) {
    return getVulkanWindowAt(index, app)->window;
}

VkSurfaceKHR getCurrentSurface(const GLFWApp app) {
    return getCurrentVulkanWindow(app)->surface;
}


void framebufferResizeCallback(GLFWwindow *window,const int width,const int height) {
    VulkanWindow *vulkanWindow = (VulkanWindow *) glfwGetWindowUserPointer(window);
    vulkanWindow->resized = true;
    vulkanWindow->extent.height = height;
    vulkanWindow->extent.width = width;
}

void createVulkanWindow(const int width, const int height, const char *title, GLFWmonitor *monitor, GLFWApp *app) {
    if (app == NULL) return;
    if (app->windows == NULL) app->windows = createUint32SizedMutableArray();

    constexpr size_t winSize = sizeof(VulkanWindow);
    VulkanWindow *vulkanWindow = malloc(winSize);
    vulkanWindow->window = glfwCreateWindow(width, height, title, monitor,NULL);
    vulkanWindow->currentFrame = 0;
    vulkanWindow->resized = false;

    addToUint32SizedMutableArray(
        winSize, // sizeof(window) should always equate to the same value roughly i.e 8
        vulkanWindow,
        app->windows
    );

    if (app->windows->count == 0) {
        app->rootWindow = 0;
        app->currentWindow = 0;
    } else app->currentWindow = app->windows->count - 1;

    glfwSetWindowUserPointer(vulkanWindow->window, vulkanWindow);
    glfwSetFramebufferSizeCallback(vulkanWindow->window, framebufferResizeCallback);
    glfwSetWindowSizeCallback(vulkanWindow->window, framebufferResizeCallback);
}

#endif //LEARNING_GLFW_APP_H
