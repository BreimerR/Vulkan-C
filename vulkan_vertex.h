//
// Created by brymher on 11/02/25.
//

#ifndef VULKAN_VERTEX_H
#define VULKAN_VERTEX_H

typedef struct Vector2D {
    float x;
    float y;
} Vector2D;

typedef struct Vector3D {
    float x; // or also r
    float y; // g
    float z; // b
} Vector3D;

typedef struct Vertex {
    Vector2D position;
    Vector3D color;
} Vertex;


void getBindingDescription(VkVertexInputBindingDescription *bindingDescription) {
    bindingDescription->binding = 0;
    bindingDescription->stride = sizeof(Vertex);
    bindingDescription->inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
}

/**
 * This section defines how the vertex data is going
 * to be read by the gpu.
 **/
void getAttributeDescriptions(VkVertexInputAttributeDescription *attributeDescriptions) {
    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(Vertex, position);

    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(Vertex, color);
}

/**
 * The VkPhysicalDeviceMemoryProperties structure has two arrays memoryTypes and memoryHeaps.
 * Memory heaps are distinct memory resources like dedicated VRAM and swap space
 * in RAM for when VRAM runs out. The different types of memory exist within these heaps.
 * Right now we'll only concern ourselves with the type of memory and not the heap
 * it comes from, but you can imagine that this can affect performance.
 * Let's first find a memory type that is suitable for the buffer itself:
 **/
uint32_t findMemoryType(
    const VkPhysicalDevice physicalDevice,
    const uint32_t typeFilter,
    const VkMemoryPropertyFlags properties
) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }
    printLn("Failed to find suitable memory type!");
    exit(1);
}

void createBuffer(
    VkBuffer *vertexBuffer,
    const VkDeviceSize bufferSize,
    VkDeviceMemory *vertexBufferMemory,
    VkMemoryRequirements *memRequirements,
    const VkBufferUsageFlags bufferUsage,
    const VkMemoryPropertyFlags memoryProperties,
    const VkPhysicalDevice physicalDevice,
    const VkDevice logicalDevice
) {
    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = bufferSize;
    bufferInfo.usage = bufferUsage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    if (vkCreateBuffer(logicalDevice, &bufferInfo, nullptr, vertexBuffer) != VK_SUCCESS) {
        printLn("failed to create vertex buffer!");
        exit(4);
    }
    vkGetBufferMemoryRequirements(logicalDevice, *vertexBuffer, memRequirements);
    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements->size;
    allocInfo.memoryTypeIndex = findMemoryType(
        physicalDevice,
        memRequirements->memoryTypeBits,
        memoryProperties
    );

    // https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator
    // memory limit is almost hit once you get to 4096 per vertex
    if (vkAllocateMemory(logicalDevice, &allocInfo, nullptr, vertexBufferMemory) != VK_SUCCESS) {
        printLn("failed to allocate vertex buffer memory!");
        exit(1);
    }

    vkBindBufferMemory(logicalDevice, *vertexBuffer, *vertexBufferMemory, 0);
}

void copyBuffer(
    VkBuffer srcBuffer,
    VkBuffer dstBuffer,
    VkDeviceSize size,
    VkCommandPool commandPool,
    VkDevice logicalDevice,
    VkQueue graphicsQueue
) {
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(logicalDevice, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    VkBufferCopy copyRegion = {};
    copyRegion.srcOffset = 0; // Optional
    copyRegion.dstOffset = 0; // Optional
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo  = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(graphicsQueue);

    vkFreeCommandBuffers(logicalDevice, commandPool, 1, &commandBuffer);
}

void createVertexBuffer(
    const VkPhysicalDevice physicalDevice,
    const VkDevice logicalDevice,
    const Uint32SizedMutableArray vertices,
    VulkanWindow *window,
    VkQueue graphicsQueue
) {
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    createBuffer(
        &stagingBuffer,
        vertices.size,
        &stagingBufferMemory,
        &window->memRequirements,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        physicalDevice,
        logicalDevice
    );

    Any data;
    vkMapMemory(logicalDevice, stagingBufferMemory, 0, vertices.size, 0, &data);
    memcpy(data, vertices.items, vertices.size);
    vkUnmapMemory(logicalDevice, stagingBufferMemory);

    createBuffer(
        &window->vertexBuffer,
        vertices.size,
        &window->vertexBufferMemory,
        &window->memRequirements,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        physicalDevice,
        logicalDevice
    );

    copyBuffer(stagingBuffer, window->vertexBuffer, vertices.size, window->commandPool, logicalDevice, graphicsQueue);

    vkDestroyBuffer(logicalDevice, stagingBuffer, nullptr);
    vkFreeMemory(logicalDevice, stagingBufferMemory, nullptr);

}
#endif //VULKAN_VERTEX_H
