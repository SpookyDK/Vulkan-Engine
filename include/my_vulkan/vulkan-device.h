#ifndef VULKAN_DEVICE_H
#define VULKAN_DEVICE_H

#include <stdbool.h>
#include <vulkan/vulkan.h>

// Struct meant to hold all values associated with a vulkan device.
//
typedef struct {
    int32_t graphicsFamily;
    int32_t presentaionFamily;
} QueueFamiliyIndices;

typedef struct {
    VkSurfaceCapabilitiesKHR capabilities;
    uint32_t formatCount;
    VkSurfaceFormatKHR *formats;
    uint32_t presentModeCount;
    VkPresentModeKHR *presentModes;
} SwapChainSupportDetails;
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
    SwapChainSupportDetails swapChainSupportDetails;

} vkdevice_attributes_t;

int find_queue_families(vkdevice_attributes_t *device_attribibutes);
int evaluate_vulkan_device(vkdevice_attributes_t *device_attribibutes);
bool check_device_extension_support(vkdevice_attributes_t *device_attribibutes);
int create_SwapChainSupportDetails(vkdevice_attributes_t *device_attribibutes);
void free_Swap_chain_support_details(SwapChainSupportDetails details);
int create_logical_device(vkdevice_attributes_t *device_attribibutes);
#endif /* VULKAN_DEVICE_H */
