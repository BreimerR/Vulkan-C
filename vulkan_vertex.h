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

void createVertexBuffer(
    const VkPhysicalDevice physicalDevice,
    const VkDevice logicalDevice,
    const Uint32SizedMutableArray vertices,
    VulkanWindow *window
) {
    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = vertices.size;
    bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    if (vkCreateBuffer(logicalDevice, &bufferInfo, nullptr, &window->vertexBuffer) != VK_SUCCESS) {
        printLn("failed to create vertex buffer!");
        exit(4);
    }
    vkGetBufferMemoryRequirements(logicalDevice, window->vertexBuffer, &window->memRequirements);

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = window->memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(
        physicalDevice,
        window->memRequirements.memoryTypeBits,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    );

    if (vkAllocateMemory(logicalDevice, &allocInfo, nullptr, &window->vertexBufferMemory) != VK_SUCCESS) {
        printLn("failed to allocate vertex buffer memory!");
        exit(1);
    }

    vkBindBufferMemory(logicalDevice, window->vertexBuffer, window->vertexBufferMemory, 0);

    Any data;
    vkMapMemory(logicalDevice, window->vertexBufferMemory, 0, bufferInfo.size, 0, &data);
    memcpy(data, vertices.items,  bufferInfo.size);
    vkUnmapMemory(logicalDevice, window->vertexBufferMemory);

}
#endif //VULKAN_VERTEX_H
