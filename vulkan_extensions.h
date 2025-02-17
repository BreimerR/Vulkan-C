//
// Created by brymher on 18/12/24.
//

#ifndef LEARNING_VULKAN_EXTENSIONS_H
#define LEARNING_VULKAN_EXTENSIONS_H

#include <vulkan/vulkan.h>
#include "glfw_app.h"
#include "malloc.h"

typedef struct GLFWVulkanExtension {
    uint32_t *count;
    VkExtensionProperties *extensions;
} GLFWVulkanExtension;

typedef struct VkValidationLayers {
    uint32_t *count;
    VkLayerProperties *properties;
} VkValidationLayers;

Uint32SizedMutableArray *getGLFWExtensions() {
    Uint32SizedMutableArray *extensions = createUint32SizedMutableArray();

    extensions->items = (Any*) glfwGetRequiredInstanceExtensions(&extensions->count);

    return extensions;
}

Uint32SizedMutableArray *getSupportedVulkanExtensions() {
    Uint32SizedMutableArray *extensions = malloc(sizeof(Uint32SizedMutableArray));
    // VkExtensionProperties
    vkEnumerateInstanceExtensionProperties(NULL, &extensions->count, NULL);

    extensions->items = (Any*) malloc(sizeof(VkExtensionProperties) * extensions->count);
    vkEnumerateInstanceExtensionProperties(NULL, &extensions->count, (VkExtensionProperties *) extensions->items);

    return extensions;
}

void populateSupportedVkValidationLayers(Uint32SizedMutableArray *layers) {
    if (layers == NULL) return;

    vkEnumerateInstanceLayerProperties(&layers->count, NULL);
    layers->size = sizeof(VkLayerProperties) * layers->count;
    layers->items = malloc(layers->size);
    vkEnumerateInstanceLayerProperties(&layers->count, (VkLayerProperties *) layers->items);
}

/*Ensure to clean up after yourself clearSupportedVkValidationLayers*/
Uint32SizedMutableArray *getSupportedVkValidationLayers() {
    Uint32SizedMutableArray *layers = malloc(sizeof(Uint32SizedMutableArray));

    populateSupportedVkValidationLayers(layers);

    return layers;
}


void clearSupportedVkValidationLayers(VkValidationLayers *layers) {
    free(layers->properties);
    free(layers);
}

/**
 * We do not clear the individual items since
 * the application uses them internally not just here.
 **/
void clearGLFWExtensions(Uint32SizedMutableArray *extensions) {
    if (extensions == NULL) return;
    free(extensions);
}

#endif //LEARNING_VULKAN_EXTENSIONS_H
