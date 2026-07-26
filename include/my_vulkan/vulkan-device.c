#include "stdbool.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
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
        vkGetPhysicalDeviceSurfaceSupportKHR(device_attribibutes->physicalDevice, i, device_attribibutes->surface, &presentSupport);
        if (presentSupport) {
            device_attribibutes->queueFamilyIndices.presentaionFamily = i;
        }
    }
    return 0;
}
int evaluate_vulkan_device(vkdevice_attributes_t *device_attribibutes) {
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

    find_queue_families(device_attribibutes);
    if (device_attribibutes->queueFamilyIndices.graphicsFamily == -1) {
        device_attribibutes->score = 0;
        return 1;
    }
    if (!check_device_extension_support(device_attribibutes)) {
        device_attribibutes->score = 0;
        return 1;
    }
    create_SwapChainSupportDetails(device_attribibutes);
    if (device_attribibutes->swapChainSupportDetails.formatCount == 0 ||
        device_attribibutes->swapChainSupportDetails.presentModeCount == 0) {
        device_attribibutes->score = 0;
        return 1;
    }
    if (device_attribibutes->features.samplerAnisotropy != VK_TRUE) {
        device_attribibutes->score = 0;
        return 1;
    }
    free_Swap_chain_support_details(device_attribibutes->swapChainSupportDetails);

    return 0;
}

const char *deviceExtensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
const uint32_t requiredExtensionCount = 1;
bool check_device_extension_support(vkdevice_attributes_t *device_attribibutes) {
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device_attribibutes->physicalDevice, NULL, &extensionCount, NULL);
    VkExtensionProperties avaiableExtensions[extensionCount];

    vkEnumerateDeviceExtensionProperties(device_attribibutes->physicalDevice, NULL, &extensionCount, avaiableExtensions);
    bool extensionsFound = true;

    for (uint32_t i = 0; i < requiredExtensionCount; i++) {
        extensionsFound = false;
        for (uint32_t j = 0; j < extensionCount; j++) {
            if (!strcmp(deviceExtensions[i], avaiableExtensions[j].extensionName)) {
                extensionsFound = true;
                break;
            }
        }
    }
    return extensionsFound;
}

int create_SwapChainSupportDetails(vkdevice_attributes_t *device_attribibutes) {
    if (device_attribibutes->surface == NULL) {
        printf("device_attribibutes->Surface ==NULL. c:%d\n", __LINE__);
        return 1;
    }

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device_attribibutes->physicalDevice, device_attribibutes->surface,
                                              &device_attribibutes->swapChainSupportDetails.capabilities);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device_attribibutes->physicalDevice, device_attribibutes->surface,
                                         &device_attribibutes->swapChainSupportDetails.formatCount, NULL);
    if (device_attribibutes->swapChainSupportDetails.formatCount != 0) {
        device_attribibutes->swapChainSupportDetails.formats =
            malloc(device_attribibutes->swapChainSupportDetails.formatCount * sizeof(VkSurfaceFormatKHR));
        vkGetPhysicalDeviceSurfaceFormatsKHR(device_attribibutes->physicalDevice, device_attribibutes->surface,
                                             &device_attribibutes->swapChainSupportDetails.formatCount,
                                             device_attribibutes->swapChainSupportDetails.formats);
    }

    vkGetPhysicalDeviceSurfacePresentModesKHR(device_attribibutes->physicalDevice, device_attribibutes->surface,
                                              &device_attribibutes->swapChainSupportDetails.presentModeCount, NULL);
    if (device_attribibutes->swapChainSupportDetails.presentModeCount != 0) {
        device_attribibutes->swapChainSupportDetails.presentModes =
            malloc(device_attribibutes->swapChainSupportDetails.presentModeCount * sizeof(VkPresentModeKHR));
        vkGetPhysicalDeviceSurfacePresentModesKHR(device_attribibutes->physicalDevice, device_attribibutes->surface,
                                                  &device_attribibutes->swapChainSupportDetails.presentModeCount,
                                                  device_attribibutes->swapChainSupportDetails.presentModes);
    }
    return 0;
}

void free_Swap_chain_support_details(SwapChainSupportDetails details) {
    free(details.presentModes);
    free(details.formats);
}

#define FAMILYCOUNT 2
int create_logical_device(vkdevice_attributes_t *device_attributes) {
    VkDeviceQueueCreateInfo queueCreateInfo[FAMILYCOUNT] = {};
    uint32_t uniqueQueues[FAMILYCOUNT] = {device_attributes->queueFamilyIndices.graphicsFamily,
                                          device_attributes->queueFamilyIndices.presentaionFamily};

    float queuePriority = 1.0f;
    for (int i = 0; i < FAMILYCOUNT; i++) {
        queueCreateInfo[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo[i].queueFamilyIndex = uniqueQueues[i];
        queueCreateInfo[i].queueCount = 1;
        queueCreateInfo[i].pQueuePriorities = &queuePriority;
    }
    VkPhysicalDeviceFeatures deviceFeatures = {0};
    deviceFeatures.samplerAnisotropy = VK_TRUE;
    deviceFeatures.sampleRateShading = VK_TRUE;
    VkDeviceCreateInfo deviceCreateInfo = {};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.pQueueCreateInfos = queueCreateInfo;
    deviceCreateInfo.queueCreateInfoCount = 2;
    deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
    deviceCreateInfo.enabledExtensionCount = requiredExtensionCount;
    deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions;
    if (vkCreateDevice(device_attributes->physicalDevice, &deviceCreateInfo, NULL, &device_attributes->device) != VK_SUCCESS) {
        printf("vkCreateDeviceInfo Failed line %d", __LINE__);
        return 1;
    }
    vkGetDeviceQueue(device_attributes->device, device_attributes->queueFamilyIndices.graphicsFamily, 0, &device_attributes->graphicsQueue);
    vkGetDeviceQueue(device_attributes->device, device_attributes->queueFamilyIndices.presentaionFamily, 0,
                     &device_attributes->presentationQueue);
    return 0;
}
