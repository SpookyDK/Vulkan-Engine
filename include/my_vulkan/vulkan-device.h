#ifndef VULKAN_DEVICE_H
#define VULKAN_DEVICE_H

#include <vulkan/vulkan.h>

// Struct meant to hold all values associated with a vulkan device.
//
typedef struct {
    int32_t graphicsFamily;
    int32_t presentaionFamily;
} QueueFamiliyIndices;
typedef struct {
    uint8_t score;
    VkPhysicalDevice physicalDevice;
    VkDevice device;
    VkQueue presentationQueue;
    VkQueue graphicsQueue;
    VkPhysicalDeviceFeatures features;
    VkPhysicalDeviceProperties properties;
    QueueFamiliyIndices queueFamilyIndices;
    uint32_t queueFamilyIndicesCount;
    VkSurfaceKHR surface;

} vkdevice_attributes_t;

int find_queue_families(vkdevice_attributes_t *device_attribibutes);
uint8_t evaluate_vulkan_device(vkdevice_attributes_t *device_attribibutes);
#endif /* VULKAN_DEVICE_H */
