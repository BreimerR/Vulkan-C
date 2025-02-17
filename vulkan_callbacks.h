//
// Created by brymher on 29/01/25.
//

#ifndef VULKAN_CALLBACKS_H
#define VULKAN_CALLBACKS_H

#include <vulkan/vulkan.h>
#include <stdlib.h>

void *pfnvkAllocationFunction(
    void *pUserData,
    size_t size,
    size_t alignment,
    VkSystemAllocationScope allocationScope) {
    return malloc(size);
}

void *pfnvkReallocationFunction(
    void *pUserData,
    void *pOriginal,
    size_t size,
    size_t alignment,
    VkSystemAllocationScope allocationScope) {
    return realloc(pOriginal, size);
}

void pfnvkFreeFunction(
    void *pUserData,
    void *pMemory) {
    printLn("VK_INFO: %s", pUserData);
    if (pMemory == nullptr) return;
    free(pMemory);
}

void fn_vkInternalAllocationNotification(
    void *pUserData,
    size_t size,
    VkInternalAllocationType allocationType,
    VkSystemAllocationScope allocationScope) {

}

void pfn_vkInternalFreeNotification(
    void *pUserData,
    size_t size,
    VkInternalAllocationType allocationType,
    VkSystemAllocationScope allocationScope) {
}

void populateDeallocationCallbacks(VkAllocationCallbacks *callbacks, Any label) {
    callbacks->pUserData = label;
    callbacks->pfnAllocation = pfnvkAllocationFunction;
    callbacks->pfnReallocation = pfnvkReallocationFunction;
    callbacks->pfnFree = pfnvkFreeFunction;
    callbacks->pfnInternalAllocation = fn_vkInternalAllocationNotification;
    callbacks->pfnInternalFree = pfn_vkInternalFreeNotification;
}

#endif //VULKAN_CALLBACKS_H
