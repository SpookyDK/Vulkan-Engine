#include "stdbool.h"
#include "stdio.h"
#include "vulkan-device.h"

int find_queue_families(vkdevice_attributes_t *device_attribibutes) {
    device_attribibutes->queueFamilyIndices.graphicsFamily = -1;
    device_attribibutes->queueFamilyIndices.presentaionFamily = -1;
    device_attribibutes->queueFamilyIndicesCount = 0;

    vkGetPhysicalDeviceQueueFamilyProperties(device_attribibutes->physicalDevice, &device_attribibutes->queueFamilyIndicesCount, NULL);
    VkQueueFamilyProperties queueFamilies[device_attribibutes->queueFamilyIndicesCount];
    vkGetPhysicalDeviceQueueFamilyProperties(device_attribibutes->physicalDevice, &device_attribibutes->queueFamilyIndicesCount,
                                             queueFamilies);
    VkBool32 presentSupport = false;
    int ret = 0;
    for (uint32_t i = 0; i < device_attribibutes->queueFamilyIndicesCount; i++) {
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            device_attribibutes->queueFamilyIndices.graphicsFamily = i;
        }
        if (device_attribibutes->surface == NULL) {
            ret = 1;
            printf("Device surface is empty, cannot query for presentsupport c:%d\n", __LINE__);
            continue;
        }
        vkGetPhysicalDeviceSurfaceSupportKHR(device_attribibutes->physicalDevice, i, device_attribibutes->surface,
                                             &presentSupport); // TODO add surface chech
        if (presentSupport) {
            device_attribibutes->queueFamilyIndices.presentaionFamily = i;
        }
    }
    return 0;
}
uint8_t evaluate_vulkan_device(vkdevice_attributes_t *device_attribibutes) {
    vkGetPhysicalDeviceProperties(device_attribibutes->physicalDevice, &device_attribibutes->properties);
    vkGetPhysicalDeviceFeatures(device_attribibutes->physicalDevice, &device_attribibutes->features);
    printf("Evaluating %s \n", device_attribibutes->properties.deviceName);
    printf("DeviceType: %d ", device_attribibutes->properties.deviceType);
    switch (device_attribibutes->properties.deviceType) {
    case 0:
        printf("Other\n");
        break;
    case 1:
        printf("Ingegrated\n");
        device_attribibutes->score = 100;
        break;
    case 2:
        printf("Discrete\n");
        device_attribibutes->score = 200;
        break;
    case 3:
        printf("Virtual\n");
        device_attribibutes->score = 100;
        break;
    case 4:
        printf("CPU\n");
        device_attribibutes->score = 10;
        break;
    default:
        printf("unknown\n");
        device_attribibutes->score = 0;
        break;
    }
    if (!device_attribibutes->features.geometryShader)
        device_attribibutes->score = 0;

    device_attribibutes->queueFamilyIndices = find_queue_families(device_attribibutes->physicalDevice);
    if (queueIndices.graphicsFamily == -1) {
        score = 0;
        return score;
    }
    if (!check_device_extension_support(_device)) {
        score = 0;
        return score;
    }
    SwapChainSupportDetails swapChainSupport = create_SwapChainSupportDetails(_device);
    if (swapChainSupport.formatCount == 0 || swapChainSupport.presentModeCount == 0) {
        score = 0;
        return score;
    }
    if (features.samplerAnisotropy != VK_TRUE) {
        score = 0;
        return score;
    }
    free_SwapChainSupportDetails(swapChainSupport);

    return score;
}
}
