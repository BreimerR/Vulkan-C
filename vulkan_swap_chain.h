//
// Created by brymher on 19/01/25.
//

#ifndef VULKAN_SWAP_CHAIN_H
#define VULKAN_SWAP_CHAIN_H

#include <vulkan/vulkan.h>

#include "vulkan.h"
#include "vulkan_frame_buffers.h"

typedef struct VkSwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    Uint32SizedMutableArray formats; // VkSurfaceFormatKHR
    Uint32SizedMutableArray presentModes;
} VkSwapChainSupportDetails;

void querySwapChainSupport(
    const VkPhysicalDevice physicalDevice,
    const VkSurfaceKHR surface,
    VkSwapChainSupportDetails *swapChainSupportDetails
) {
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &swapChainSupportDetails->capabilities);

    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &swapChainSupportDetails->formats.count, NULL);

    if (swapChainSupportDetails->formats.count > 0) {
        printLn("Swap chain supports %d formats", swapChainSupportDetails->formats.count);
        swapChainSupportDetails->formats.items = malloc(
            sizeof(VkSurfaceFormatKHR) * swapChainSupportDetails->formats.count);

        vkGetPhysicalDeviceSurfaceFormatsKHR(
            physicalDevice,
            surface,
            &swapChainSupportDetails->formats.count,
            (VkSurfaceFormatKHR *) swapChainSupportDetails->formats.items
        );
    } else printLn("Surface formats don't exit");

    vkGetPhysicalDeviceSurfacePresentModesKHR(
        physicalDevice,
        surface,
        &swapChainSupportDetails->presentModes.count,
        NULL
    );

    if (swapChainSupportDetails->presentModes.count > 0) {
        printLn("Swap chain supports %d present modes", swapChainSupportDetails->presentModes.count);
        swapChainSupportDetails->presentModes.items = malloc(
            sizeof(VkPresentModeKHR) * swapChainSupportDetails->presentModes.count);

        vkGetPhysicalDeviceSurfacePresentModesKHR(
            physicalDevice,
            surface,
            &swapChainSupportDetails->presentModes.count,
            (VkPresentModeKHR *) swapChainSupportDetails->presentModes.items
        );
    } else printLn("Surface presentModes don't exit");
}

void populateVkSwapchainCreateInfoKHR(
    VkSwapchainCreateInfoKHR *createInfo,
    const VkSurfaceFormatKHR surfaceFormat,
    const VkExtent2D extent,
    const uint32_t imageCount,
    const VkSurfaceKHR surface,
    const VkPresentModeKHR presentMode,
    const VkSurfaceCapabilitiesKHR surfaceCapabilities,
    uint32_t familyIndices[2]
) {
    createInfo->sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo->surface = surface;
    createInfo->minImageCount = imageCount;
    createInfo->imageFormat = surfaceFormat.format;
    createInfo->imageColorSpace = surfaceFormat.colorSpace;
    createInfo->imageExtent = extent;
    createInfo->imageArrayLayers = 1;
    createInfo->imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    createInfo->oldSwapchain = VK_NULL_HANDLE;
    createInfo->presentMode = presentMode;
    createInfo->clipped = VK_TRUE;
    createInfo->compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo->preTransform = surfaceCapabilities.currentTransform;
    if (familyIndices[0] != familyIndices[1]) {
        createInfo->imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo->queueFamilyIndexCount = 2;
        createInfo->pQueueFamilyIndices = familyIndices;
    } else {
        createInfo->imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo->queueFamilyIndexCount = 0; // Optional
        createInfo->pQueueFamilyIndices = nullptr; // Optional
    }
}


bool isSurfaceFormatWithSRGBSupport(const Uint32SizedMutableArray array, const uint32_t index) {
    const auto availableFormat = ((VkSurfaceFormatKHR *) array.items)[index];
    if (
        availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
        availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
    ) {
        return true;
    }

    return false;
}

int selectSurfaceFormatIndex(const Uint32SizedMutableArray formats) {
    int acceptedSurfaceFormatIndex = findFirstIndexInUint32SizedMutableArray(
        formats,
        isSurfaceFormatWithSRGBSupport
    );
    if (acceptedSurfaceFormatIndex == -1) acceptedSurfaceFormatIndex = 0;

    return acceptedSurfaceFormatIndex;
}

bool isSurfacePresentModeMailBoxKHR(const Uint32SizedMutableArray array, const uint32_t index) {
    const auto availablePresentMode = ((VkPresentModeKHR *) array.items)[index];
    if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) return true;

    return false;
}

VkPresentModeKHR selectPresentModeIndex(const Uint32SizedMutableArray presentModes) {
    const int acceptedPresentMode = findFirstIndexInUint32SizedMutableArray(
        presentModes,
        isSurfacePresentModeMailBoxKHR
    );

    if (acceptedPresentMode == -1) return VK_PRESENT_MODE_FIFO_KHR;

    return ((VkPresentModeKHR *) presentModes.items)[acceptedPresentMode];
}

uint32_t uint32Clamp(const uint32_t x, const uint32_t min, const uint32_t max) {
    if (x < min) return min;
    if (x > max) return max;
    return x;
}

void updateSwapExtent(GLFWwindow *window, const VkSurfaceCapabilitiesKHR capabilities, VkExtent2D *actualExtent) {
    if (capabilities.currentExtent.width != UINT32_MAX) {
        actualExtent->width = capabilities.currentExtent.width;
        actualExtent->height = capabilities.currentExtent.height;
    } else {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        actualExtent->width = uint32Clamp(
            actualExtent->width,
            capabilities.minImageExtent.width,
            capabilities.maxImageExtent.width
        );
        actualExtent->height = uint32Clamp(
            actualExtent->height,
            capabilities.minImageExtent.height,
            capabilities.maxImageExtent.height
        );
    }
}

void populateVulkanWindowSwapChainImages(VulkanWindow *vkWindow, const VkDevice logicalDevice) {
    vkGetSwapchainImagesKHR(logicalDevice, vkWindow->swapChain, &vkWindow->swapChainImages.count, nullptr);
    printLn("Found %d swap chain images", vkWindow->swapChainImages.count);
    vkWindow->swapChainImages.items = malloc(sizeof(VkImage) * vkWindow->swapChainImages.count);

    vkGetSwapchainImagesKHR(
        logicalDevice,
        vkWindow->swapChain,
        &vkWindow->swapChainImages.count,
        (VkImage *) vkWindow->swapChainImages.items
    );
}

void createSwapChain(
    VulkanWindow *vkWindow,
    const VkDevice logicalDevice,
    const VkSwapChainSupportDetails swapChainSupportDetails,
    VkSwapchainCreateInfoKHR createInfo,
    const uint32_t queueFamilyIndex,
    const uint32_t presentFamilyIndex
) {
    const int acceptedSurfaceFormatIndex = selectSurfaceFormatIndex(swapChainSupportDetails.formats);

    printLn("Found surface format with index %d", acceptedSurfaceFormatIndex);

    const auto surfaceFormat =
            ((VkSurfaceFormatKHR *) swapChainSupportDetails.formats.items)[acceptedSurfaceFormatIndex];
    vkWindow->swapChainImageFormat = surfaceFormat.format;

    auto const presentMode = selectPresentModeIndex(swapChainSupportDetails.presentModes);

    printLn("Selected present mode %d", presentMode);

    vkWindow->extent.height = 0;
    vkWindow->extent.width = 0;

    updateSwapExtent(vkWindow->window, swapChainSupportDetails.capabilities, &vkWindow->extent);

    printLn("Window height is %d width %d", vkWindow->extent.height, vkWindow->extent.width);

    uint32_t imageCount = swapChainSupportDetails.capabilities.minImageCount + 1;

    printLn("Setting the image count to %d", imageCount);

    if (swapChainSupportDetails.capabilities.maxImageCount > 0 && imageCount > swapChainSupportDetails.capabilities.
        maxImageCount) {
        imageCount = swapChainSupportDetails.capabilities.maxImageCount;
    }

    uint32_t familyIndices[2] = {presentFamilyIndex, queueFamilyIndex};
    populateVkSwapchainCreateInfoKHR(
        &createInfo,
        surfaceFormat,
        vkWindow->extent,
        imageCount,
        vkWindow->surface,
        presentMode,
        swapChainSupportDetails.capabilities,
        familyIndices
    );

    if (vkCreateSwapchainKHR(logicalDevice, &createInfo, NULL, &vkWindow->swapChain) != VK_SUCCESS) {
        printLn("Failed to create swap chain!");
        exit(FAILED_TO_CREATE_SWAP_CHAIN);
    }

    populateVulkanWindowSwapChainImages(vkWindow, logicalDevice);
}

void createImageViews(VulkanWindow *vkWindow, const VkDevice logicalDevice) {
    vkWindow->swapChainImagesViews.count = vkWindow->swapChainImages.count;
    vkWindow->swapChainImagesViews.size = sizeof(VkImageView) * vkWindow->swapChainImages.count;
    vkWindow->swapChainImagesViews.items = malloc(vkWindow->swapChainImagesViews.size);
    for (uint32_t i = 0; i < vkWindow->swapChainImages.count; i++) {
        const VkImage vkImage = ((VkImage *) vkWindow->swapChainImages.items)[i];

        VkImageViewCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = vkImage;
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = vkWindow->swapChainImageFormat;

        /*
         * The components field allows you to swizzle the color channels around.
         * For example, you can map all the channels to the red channel for a monochrome texture.
         * You can also map constant values of 0 and 1 to a channel.
         * In our case we'll stick to the default mapping.
         */
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

        /*
         * The subresourceRange field describes what the image's purpose is
         * and which part of the image should be accessed.
         * Our images will be used as color targets
         * without any mipmapping levels or multiple layers.
         */
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        VkImageView *imageView = &((VkImageView *) vkWindow->swapChainImagesViews.items)[i];
        if (vkCreateImageView(logicalDevice, &createInfo, nullptr, imageView) != VK_SUCCESS) {
            printLn("Failed to create image view!");
            exit(FAILED_TO_CREATE_SWAP_CHAIN_IMAGE_VIEWS);
        }

        printLn("Created swap chain image view %d", i);
    }
}

void cleanUpSwapChain(VkDevice logicalDevice, VulkanWindow *vulkanWindow) {
    VkAllocationCallbacks callbacks = {
        .pUserData = "Smile More",
        .pfnAllocation = pfnvkAllocationFunction,
        .pfnReallocation = pfnvkReallocationFunction,
        .pfnFree = pfnvkFreeFunction,
        .pfnInternalAllocation = fn_vkInternalAllocationNotification,
        .pfnInternalFree = pfn_vkInternalFreeNotification
    };

    for (uint32_t j = 0; j < vulkanWindow->swapChainImagesViews.count; j++) {
        if (vulkanWindow->swapChainImagesViews.items == nullptr) continue;
        else printLn("Image views aren't empty");
        VkImageView imageView = ((VkImageView *) vulkanWindow->swapChainImagesViews.items)[j];
        if (&imageView == nullptr) continue;
        else printLn("Current image view isn't empty");

        callbacks.pUserData = "vkDestroyImageVies";

        vkDestroyImageView(logicalDevice, imageView, &callbacks);

        const VkFramebuffer frameBuffer = ((VkFramebuffer *) vulkanWindow->swapChainFrameBuffers.items)[j];
        callbacks.pUserData = "vkDestroyFramebuffer";
        vkDestroyFramebuffer(logicalDevice, frameBuffer, &callbacks);
    }

    callbacks.pUserData = "vkDestroySwapchainKHR";
    vkDestroySwapchainKHR(logicalDevice, vulkanWindow->swapChain, &callbacks);
}

void prepareSwapChain(GLFWApp *app, VkSwapChainSupportDetails *swapChainSupportDetails) {
    const VkSwapchainCreateInfoKHR createInfo = {};

    createSwapChain(
        getCurrentVulkanWindow(*app),
        app->logicalDevice,
        *swapChainSupportDetails,
        createInfo,
        app->queueFamilyIndex,
        app->presentFamilyIndex
    );
    createImageViews(getCurrentVulkanWindow(*app), app->logicalDevice);
    createFrameBuffers(app->logicalDevice, getCurrentVulkanWindow(*app));
}

void recreateSwapChain(GLFWApp *app) {
    VulkanWindow *vulkanWindow = getCurrentVulkanWindow(*app);


    int width = 0, height = 0;
    glfwGetFramebufferSize(vulkanWindow->window, &width, &height);
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(vulkanWindow->window, &width, &height);
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(app->logicalDevice);


    cleanUpSwapChain(app->logicalDevice, getCurrentVulkanWindow(*app));
    VkSwapChainSupportDetails swapChainSupportDetails = {
        .presentModes = {
            .count = 0
        },
        .formats = {
            .count = 0
        }
    };
    querySwapChainSupport(
        ((VkPhysicalDevice *) app->physicalDevices->items)[app->currentPhysicalDevice],
        vulkanWindow->surface,
        &swapChainSupportDetails
    );
    prepareSwapChain(app, &swapChainSupportDetails);
}

#endif //VULKAN_SWAP_CHAIN_H
