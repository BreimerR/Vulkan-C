//
// Created by brymher on 18/12/24.
//

#ifndef LEARNING_VULKAN_H
#define LEARNING_VULKAN_H

#include <limits.h>

#include "constants.h"
#include <vulkan/vulkan.h>
#include <stdlib.h>
#include <string.h>
#include "io.h"
#include "glfw_app.h"
#include "array.h"
#include "vulkan_callbacks.h"
#include "vulkan_extensions.h"
#include "vulkan_swap_chain.h"
#include "vulkan_graphics_pipeline.h"
#include "vulkan_frame_buffers.h"
#include "vulkan_command_buffers.h"

typedef bool (*VkDeviceSelectionCriteria)(VkPhysicalDevice, VkPhysicalDeviceProperties, VkPhysicalDeviceFeatures,
                                          Uint32SizedMutableArray);


VkInstanceCreateInfo *createVkInstanceCreateInfo(
    //const void *pNext,
    // VkInstanceCreateFlags flags,
    const VkApplicationInfo *pApplicationInfo,
    const uint32_t enabledExtensionCount,
    const char *const *ppEnabledExtensionNames,
    const uint32_t enabledLayerCount,
    const char **ppEnabledLayerNames
) {
    VkInstanceCreateInfo *vkCreateInfo = malloc(sizeof(VkInstanceCreateInfo));

    vkCreateInfo->sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            vkCreateInfo->pApplicationInfo = pApplicationInfo,
            vkCreateInfo->enabledExtensionCount = enabledExtensionCount,
            vkCreateInfo->ppEnabledExtensionNames = ppEnabledExtensionNames,
            vkCreateInfo->enabledLayerCount = enabledLayerCount;
    vkCreateInfo->ppEnabledLayerNames = ppEnabledLayerNames;

    return vkCreateInfo;
}

void assignVulkanApplicationInfo(
    VkApplicationInfo *appInfo,
    const VkStructureType sType,
    const void *pNext,
    const char *pApplicationName,
    const uint32_t applicationVersion,
    const char *pEngineName,
    const uint32_t engineVersion,
    const uint32_t apiVersion
) {
    appInfo->sType = sType;
    appInfo->pNext = pNext;
    appInfo->pApplicationName = pApplicationName;
    appInfo->applicationVersion = applicationVersion;
    appInfo->pEngineName = pEngineName;
    appInfo->engineVersion = engineVersion;
    appInfo->apiVersion = apiVersion;
}

VkApplicationInfo *createVulkanApplicationInfo(
    const VkStructureType sType,
    const void *pNext,
    const char *pApplicationName,
    const uint32_t applicationVersion,
    const char *pEngineName,
    const uint32_t engineVersion,
    const uint32_t apiVersion
) {
    VkApplicationInfo *appInfo = malloc(sizeof(VkApplicationInfo));
    appInfo->sType = sType;
    appInfo->pNext = pNext;
    appInfo->pApplicationName = pApplicationName;
    appInfo->applicationVersion = applicationVersion;
    appInfo->pEngineName = pEngineName;
    appInfo->engineVersion = engineVersion;
    appInfo->apiVersion = apiVersion;
    return appInfo;
}


void buildVulkanInstance(
    GLFWApp *app,
    VkApplicationInfo *appInfo,
    Uint32SizedMutableArray *glfwExtensions,
    const Uint32SizedMutableArray enabledValidationLayers,
    VkDebugUtilsMessengerCreateInfoEXT *debugCreateInfo
) {
    VkInstanceCreateInfo *vkCreateInfo = createVkInstanceCreateInfo(
        appInfo,
        glfwExtensions->count,
        (const char **) glfwExtensions->items,
        enabledValidationLayers.count,
        (const char **) enabledValidationLayers.items
    );


    vkCreateInfo->pNext = debugCreateInfo;
    app->vkInstance = malloc(sizeof(VkInstance));

    if (vkCreateInstance(vkCreateInfo, NULL, app->vkInstance) != VK_SUCCESS) {
        printErrLn("Failed to create Vulkan Instance");
        exit(-1);
    } else printLn("CREATED VULKAN INSTANCE CORRECTLY");
}

bool compareVkExtensions(Any a, Uint32SizedMutableArray array, uint32_t index) {
    Any b = ((VkExtensionProperties *) array.items)[index].extensionName;
    printLn("\n supported extension %s \t requested extension %s", (char *) a, (char *) b);
    if (strcmp((char *) a, (char *) b) == 0) return true;

    return false;
}

Any getVkLayerPropertyAtIndex(const uint32_t index, const Uint32SizedMutableArray array) {
    return ((VkLayerProperties *) array.items)[index].layerName;
}

bool compareVkLayerNames(Any first, const Uint32SizedMutableArray array, const uint32_t index) {
    Any second = ((VkLayerProperties *) array.items)[index].layerName;
    if (strcmp((char *) first, (char *) second) == 0) return true;

    return false;
}

/**#Documentation
 * @param messageSeverity VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT
 *                        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT
 *@param messageType      VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
 *                        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT
 **/
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    const VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    const VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
    void *pUserData
) {
    const char *severity = "VK_MESSAGE";

    if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
        severity = "VK_VERBOSE";
    else if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
        severity = "VK_INFO";
    else if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
        severity = "VK_WARNING";
    else if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) severity = "VK_ERROR";

    printLn("%s: %s", severity, pCallbackData->pMessage);

    return VK_FALSE;
}

void addSupportedVulkanLayerExtensions(
    Uint32SizedMutableArray *enabledLayerExtensions,
    const Uint32SizedMutableArray requestedValidationLayers
) {
    if (requestedValidationLayers.count < 1) return;

    Uint32SizedMutableArray availableLayersArray = {
        .count = 0,
        .size = 0
    };
    populateSupportedVkValidationLayers(&availableLayersArray);

    for (int i = 0; i < requestedValidationLayers.count; ++i) {
        const char *layerName = ((VkLayerProperties *) availableLayersArray.items)[i].layerName;
        const char *requestedLayerName = (char *) requestedValidationLayers.items[i];
        const int index = getFirstIndexOfItemInUint32SizedMutableArray(
            (Any) layerName,
            availableLayersArray,
            compareVkLayerNames
        );

        if (index < 0) printLn("Requested Validation layer %s is not supported.", requestedLayerName);
        else {
            printLn("Enabled layer name %s", layerName);
            addToUint32SizedMutableArray(sizeof(layerName), (Any) layerName, enabledLayerExtensions);
        }
    }
}

void addSupportedVulkanExtensions(
    Uint32SizedMutableArray *glfwExtensions,
    const Uint32SizedMutableArray enabledExtensions
) {
    Uint32SizedMutableArray *properties = getSupportedVulkanExtensions();

    for (int i = 0; i < enabledExtensions.count; ++i) {
        const auto candidateExtension = (char *) enabledExtensions.items[i];
        const int extensionIndex = getFirstIndexOfItemInUint32SizedMutableArray(
            candidateExtension,
            *properties,
            compareVkExtensions
        );

        if (extensionIndex > 0) {
            addToUint32SizedMutableArray(
                strlen(candidateExtension) * sizeof(char),
                candidateExtension,
                glfwExtensions
            );
            // TODO Disabled debug code
            //
            printLn("Extension to enable is %s\n", candidateExtension);
        }
    }

    free(properties);
}

VkResult CreateDebugUtilsMessengerEXT(
    VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
    const VkAllocationCallbacks *pAllocator,
    VkDebugUtilsMessengerEXT *pDebugMessenger
) {
    const auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance,
        "vkCreateDebugUtilsMessengerEXT");
    if (func != NULL) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void setUpDebug(GLFWApp app, VkDebugUtilsMessengerCreateInfoEXT *createInfo) {
    app.debugMessenger = malloc(sizeof(VkDebugUtilsMessengerEXT));

    if (VK_SUCCESS != CreateDebugUtilsMessengerEXT(
            *app.vkInstance,
            createInfo,
            NULL,
            app.debugMessenger
        )) {
        printErrLn("Failed to create error message handler");
        exit(39);
    } else printLn("Created error message handler");
}

void populateDebugMessenger(VkDebugUtilsMessengerCreateInfoEXT *createInfo) {
    createInfo->sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo->messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                  VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
                                  VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                  VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo->messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                              VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                              VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo->pfnUserCallback = debugCallback;
    createInfo->pUserData = NULL; // Optional
}

// Should be internal not for use externally
void initVulkan(
    GLFWApp *app,
    const Uint32SizedMutableArray enabledExtensions,
    const Uint32SizedMutableArray requestedValidationLayers
) {
    VkApplicationInfo *appInfo = createVulkanApplicationInfo(
        VK_STRUCTURE_TYPE_APPLICATION_INFO,
        NULL,
        app->name,
        VK_MAKE_VERSION(1, 0, 0),
        "KUI ENGINE",
        VK_MAKE_VERSION(1, 0, 0),
        VK_API_VERSION_1_0
    );

    Uint32SizedMutableArray *glfwExtensions = getGLFWExtensions();
    Uint32SizedMutableArray enabledValidationLayers = {.size = 0, .count = 0};

    // VkExtensionProperties
    addSupportedVulkanExtensions(glfwExtensions, enabledExtensions);
    addSupportedVulkanLayerExtensions(&enabledValidationLayers, requestedValidationLayers);

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {};

    populateDebugMessenger(&debugCreateInfo);
    buildVulkanInstance(app, appInfo, glfwExtensions, enabledValidationLayers, &debugCreateInfo);
    setUpDebug(*app, &debugCreateInfo);
    // Required for clean up
    free(appInfo);
    clearGLFWExtensions(glfwExtensions);
}

void populateQueueFamilies(const VkPhysicalDevice device, Uint32SizedMutableArray *queueFamilies) {
    if (queueFamilies->count > 0) {
        queueFamilies->count = 0;
        queueFamilies->size = 0;
        free(queueFamilies->items);
    }
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilies->count, NULL);

    if (queueFamilies->count == 0) return;

    queueFamilies->items = malloc(sizeof(VkQueueFamilyProperties) * queueFamilies->count);

    vkGetPhysicalDeviceQueueFamilyProperties(
        device,
        &queueFamilies->count,
        (VkQueueFamilyProperties *) queueFamilies->items
    );
}

void enumeratePhysicalDevices(GLFWApp *app, const VkInstance *vkInstance) {
    vkEnumeratePhysicalDevices(*vkInstance, &app->physicalDevices->count, NULL);

    if (app->physicalDevices->count == 0) {
        printLn("Failed to get usable GPU physical devices");
        exit(29);
    } else printLn("Found %d physical devices", app->physicalDevices->count);

    vkEnumeratePhysicalDevices(
        *vkInstance,
        &app->physicalDevices->count,
        (VkPhysicalDevice *) app->physicalDevices->items
    );

    printLn("Done enumerating physical devices");
}

void selectPresentMode(
    GLFWApp *app,
    const Uint32SizedMutableArray queueFamilies,
    const VkPhysicalDevice physicalDevice,
    const VkSurfaceKHR surface
) {
    for (uint32_t j = 0; j < queueFamilies.count; j++) {
        printLn("At Queue family  %d", j);

        const auto queueFamilyProperties = ((VkQueueFamilyProperties *) queueFamilies.items)[j];

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, j, surface, &presentSupport);

        /**
         * Nevertheless, you could add logic to explicitly prefer a physical device
         * that supports drawing and presentation in the same queue for improved performance.
         **/
        if (presentSupport) {
            app->presentFamilyIndex = j;
            printLn("Selecting present mode %d", j);
        }

        if (queueFamilyProperties.queueFlags & VK_QUEUE_GRAPHICS_BIT) app->queueFamilyIndex = j;

        if (app->presentFamilyIndex != -1 && app->queueFamilyIndex != -1) break;
    }

    printLn("Present Family Index %d, queueFamilyIndex %d", app->presentFamilyIndex, app->queueFamilyIndex);
}

void selectPhysicalDevice(
    GLFWApp *app,
    const VkSurfaceKHR surface,
    const VkDeviceSelectionCriteria checkDevice,
    const Uint32SizedMutableArray physicalDevices,
    VkSwapChainSupportDetails *swapChainSupportDetails,
    const Uint32SizedMutableArray expectedDeviceExtensions,
    Uint32SizedMutableArray *queueFamilies
) {
    VkPhysicalDeviceFeatures deviceFeatures = {};
    VkPhysicalDeviceProperties deviceProperties = {};

    for (uint32_t i = 0; i < physicalDevices.count; i++) {
        const VkPhysicalDevice physicalDevice = ((VkPhysicalDevice *) physicalDevices.items)[i];

        vkGetPhysicalDeviceFeatures(physicalDevice, &deviceFeatures);
        vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);

        populateQueueFamilies(physicalDevice, queueFamilies);

        printLn("Found %d queue families for device %s", queueFamilies->count, deviceProperties.deviceName);
        selectPresentMode(
            app,
            *queueFamilies,
            physicalDevice,
            surface
        );

        printLn("Queue family index is %d and present family %d", app->queueFamilyIndex, app->presentFamilyIndex);

        if (checkDevice(physicalDevice, deviceProperties, deviceFeatures, expectedDeviceExtensions)) {
            if (app->queueFamilyIndex == -1 || app->presentFamilyIndex == -1) {
                printLn(
                    "Present family %d or queue family %d is invalid. Neither can be -1",
                    app->queueFamilyIndex,
                    app->presentFamilyIndex
                );
                exit(1);
            }

            // It's important to query swap chain details after device extensions support is done.
            querySwapChainSupport(physicalDevice, surface, swapChainSupportDetails);
            if (uInt32SizedArrayIsEmpty(swapChainSupportDetails->formats)) {
                printLn("Swap chain support formats is empty for device %d: %s", i, deviceProperties.deviceName);
                continue;
            }
            if (uInt32SizedArrayIsEmpty(swapChainSupportDetails->presentModes)) {
                printLn("Swap chain support presentModes is empty for device %d: %s", i, deviceProperties.deviceName);
                continue;
            }

            app->currentPhysicalDevice = i;
            break;
        }
    }

    if (app->currentPhysicalDevice == -1) {
        printLn("Failed to select a physical device");
        exit(FAILED_TO_SELECT_PHYSICAL_DEVICE);
    }

    printLn("Selected physical device %d: %s", app->currentPhysicalDevice, deviceProperties.deviceName);
}

bool checkSwapChainSupport(
    const VkPhysicalDevice physicalDevice,
    const Uint32SizedMutableArray expectedDeviceExtensions
) {
    Uint32SizedMutableArray deviceExtensions = {
        .count = -1,
        .size = 0
    };

    vkEnumerateDeviceExtensionProperties(physicalDevice, NULL, &deviceExtensions.count, NULL);
    if (deviceExtensions.count <= 0) {
        // This is just here for the IDE
        printLn("Device does not support required device extensions");
        return false;
    }
    printLn("Found %d device extensions", deviceExtensions.count);

    deviceExtensions.items = malloc(sizeof(VkExtensionProperties) * deviceExtensions.count);
    vkEnumerateDeviceExtensionProperties(
        physicalDevice,
        NULL,
        &deviceExtensions.count,
        (VkExtensionProperties *) deviceExtensions.items
    );

    uint32_t i = 0;
    while (i < expectedDeviceExtensions.count) {
        bool found = false;
        for (uint32_t j = 0; j < deviceExtensions.count; j++) {
            VkExtensionProperties existingExtension = ((VkExtensionProperties *) deviceExtensions.items)[j];
            char *requestedExtensionName = ((char **) expectedDeviceExtensions.items)[i];
            int match = strcmp(requestedExtensionName, existingExtension.extensionName);
            //printLn("Comparing existing extension (%s) to %s = %d", existingExtension.extensionName, requestedExtensionName, match);
            if (match == 0) found = true;
        }

        if (found) {
            i++;
        } else break;
    }

    return i == expectedDeviceExtensions.count;
}

bool isDisplayEnabledDevice(
    const VkPhysicalDevice physicalDevice, // Might be need to remove this
    const VkPhysicalDeviceProperties deviceProperties,
    const VkPhysicalDeviceFeatures deviceFeatures,
    const Uint32SizedMutableArray expectedDeviceExtensions
) {
    const bool result = deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && deviceFeatures.
                        geometryShader;

    return result && checkSwapChainSupport(physicalDevice, expectedDeviceExtensions);;
}


void createLogicalDevice(GLFWApp *app, const Uint32SizedMutableArray expectedDeviceExtensions) {
    VkPhysicalDeviceFeatures deviceFeatures = {};

    Uint32SizedMutableArray indices = {
        .count = 2,
        .items = (Any*) (uint32_t []){
            app->queueFamilyIndex,
            app->presentFamilyIndex
        }
    };

    Uint32SizedMutableArray queueCreateInfos = {
        .count = 0
    };

    resizeUint32SizedMutableArray(&queueCreateInfos, indices.count, sizeof(VkDeviceQueueCreateInfo));

    for (uint32_t i = 0; i < queueCreateInfos.count; i++) {
        float queuePriority = 1.0f;
        uint32_t queueFamilyIndex = ((uint32_t *) indices.items)[i];
        printLn("Adding index %d, %d", queueFamilyIndex, i);
        ((VkDeviceQueueCreateInfo *) queueCreateInfos.items)[i] = (VkDeviceQueueCreateInfo){
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = queueFamilyIndex,
            .queueCount = 1,
            .pQueuePriorities = &queuePriority
        };
    }

    printLn("Done adding VkDeviceQueueCreateInfo create info");

    const VkDeviceCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pQueueCreateInfos = ((VkDeviceQueueCreateInfo *) queueCreateInfos.items),
        .queueCreateInfoCount = 1,
        .pEnabledFeatures = &deviceFeatures,
        //.enabledLayerCount = 0,
        .enabledExtensionCount = expectedDeviceExtensions.count,
        .ppEnabledExtensionNames = (char **) expectedDeviceExtensions.items
    };

    const VkPhysicalDevice physicalDevice = ((VkPhysicalDevice *) app->physicalDevices->items)[app->
        currentPhysicalDevice];
    if (vkCreateDevice(physicalDevice, &createInfo,NULL, &app->logicalDevice) != VK_SUCCESS) {
        printLn("Failed to create logical device");
        exit(LOGICAL_DEVICE_CREATION_FAILED);
    }

    printLn("Logical device created");

    if (app->logicalDevice == VK_NULL_HANDLE) {
        printLn("Failed to create logical device");
        exit(LOGICAL_DEVICE_CREATION_FAILED);
    }

    if (app->queueFamilyIndex == -1 || app->presentFamilyIndex == -1) {
        printLn(
            "Present family %d or queue family %d is invalid. Neither can be -1",
            app->queueFamilyIndex,
            app->presentFamilyIndex
        );
        exit(1);
    }

    vkGetDeviceQueue(app->logicalDevice, app->queueFamilyIndex, 0, &app->graphicsQueue);

    if (app->graphicsQueue == VK_NULL_HANDLE) {
        printLn("Failed to create graphics queue");
        exit(1);
    } else printLn("Created graphics queue");

    vkGetDeviceQueue(app->logicalDevice, app->presentFamilyIndex, 0, &(app->presentQueue));

    if (app->presentQueue == VK_NULL_HANDLE) {
        printLn("Presentation queue not found %d", app->presentQueue);
        exit(1);
    } else printLn("Created presentation queue");

    printLn("Created Logical Device for device %d and graphics queue", app->currentPhysicalDevice);
}

void createSurface(VkSurfaceKHR *ppSurface, GLFWwindow *window, const VkInstance vkInstance) {
    if (window == NULL) {
        printLn("Window is null");
        exit(GLFW_WINDOW_SURFACE_CREATION_FAILED);
    }
    if (glfwCreateWindowSurface(vkInstance, window, NULL, ppSurface) != VK_SUCCESS) {
        printLn("Failed to create window surface");
        exit(GLFW_WINDOW_SURFACE_CREATION_FAILED);
    }
}

void prepareDevices(GLFWApp *app, VkSwapChainSupportDetails *swapChainSupportDetails) {
    const Uint32SizedMutableArray expectedDeviceExtensions = {
        .count = 1,
        .items = (Any*) (char *[]){
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
        }
    };


    if (app->vkInstance == NULL) return;

    enumeratePhysicalDevices(app, app->vkInstance);
    Uint32SizedMutableArray queueFamilies = {
        .count = 0,
        .size = 0,
        .items = nullptr
    };
    selectPhysicalDevice(
        app,
        getCurrentSurface(*app),
        isDisplayEnabledDevice,
        *app->physicalDevices,
        swapChainSupportDetails,
        expectedDeviceExtensions,
        &queueFamilies
    );
    createLogicalDevice(app, expectedDeviceExtensions);
}

/*
 * Creates the first window And the items related to it
 */
void prepareVulkanApp(GLFWApp *app) {
    if (app->vkInstance == NULL) return;
    createVulkanWindow(600, 800, "Testing Window Drawing", nullptr, app);

    VulkanWindow *currentVulkanWindow = getCurrentVulkanWindow(*app);
    createSurface(&currentVulkanWindow->surface, (GLFWwindow *) currentVulkanWindow->window, *app->vkInstance);
    VkSwapChainSupportDetails swapChainSupportDetails = {
        .presentModes = {
            .count = 0
        },
        .formats = {
            .count = 0
        }
    };
    prepareDevices(app, &swapChainSupportDetails);
    prepareSwapChain(app, &swapChainSupportDetails);

    initVulkanGraphicsPipeline(
        app->logicalDevice,
        getCurrentVulkanWindow(*app)
    );

    initCommandBuffers(
        ((VkPhysicalDevice *) app->physicalDevices->items)[app->currentPhysicalDevice],
        app->logicalDevice,
        getCurrentVulkanWindow(*app),
        app->queueFamilyIndex,
        app->graphicsQueue
    );
}

// issue when pAllocator is passed. Even if it's null the code will throw a 139
void destroyDebugUtilsMessageExt(const GLFWApp app) {
    return;
    const auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(
        *app.vkInstance,
        "vkDestroyDebugUtilsMessengerEXT");

    VkAllocationCallbacks pAllocator = {};
    populateDeallocationCallbacks(&pAllocator, "destroyDebugUtilsMessageExt");
    if (func != NULL && *app.vkInstance != NULL && *app.debugMessenger != NULL)
        func(*app.vkInstance, *app.debugMessenger, &pAllocator);
}

void cleanUpVulkan(const GLFWApp app) {
    destroyDebugUtilsMessageExt(app);
    VkAllocationCallbacks pAllocator = {};
    populateDeallocationCallbacks(&pAllocator, "vkDestroyDevice");
    vkDestroyDevice(app.logicalDevice, &pAllocator);

    VkAllocationCallbacks pAllocatorDestroyInstance = {};
    populateDeallocationCallbacks(&pAllocatorDestroyInstance, "vkDestroyInstance");
    vkDestroyInstance(*app.vkInstance, &pAllocatorDestroyInstance);
}


#endif //LEARNING_VULKAN_H
