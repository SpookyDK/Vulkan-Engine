// This is my attempt at learning to draw a triangle in vulkan over the summer holidays
// Using the superior language of C over C++bloat
//
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 800
GLFWwindow *window;

VkInstance instance;

// List of required Vulkan validation layers.
const char *validationlayers[] = {"VK_LAYER_KHRONOS_validation"};
const size_t validationLayerCount = 1;
// #ifdef NDEBUG
// const bool enableValidationLayers = false;
// #else
const bool enableValidationLayers = true;
// #endif
//
//
VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
VkDevice device = VK_NULL_HANDLE;
VkQueue graphicsQueue = VK_NULL_HANDLE;
VkQueue presentationQueue = VK_NULL_HANDLE;
VkSurfaceKHR surface;
VkSwapchainKHR swapChain;
VkImage *swapChainImages;
uint32_t swapChainImageCount = 0;
VkFormat swapChainImageFormat;
VkExtent2D swapChainExtent;
bool check_validation_layer_support() {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, NULL);
    VkLayerProperties avaiableLayers[layerCount];
    vkEnumerateInstanceLayerProperties(&layerCount, avaiableLayers);

    printf("Layercount is = %d", layerCount);
    for (int i = 0; i < validationLayerCount; i++) {
        bool layerFound = false;
        for (int j = 0; j < layerCount; j++) {
            printf("%s\n", avaiableLayers[j].layerName);
            if (strcmp(validationlayers[i], avaiableLayers[j].layerName) == 0) {
                layerFound = true;
                break;
            }
        }
        if (layerFound == false) {
            return false;
        }
    }
    return true;
}
int init_window() {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "UnrealEngine25", NULL, NULL);
    return 0;
}
int deinit_window() {
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

int create_instance() {

    VkApplicationInfo appInfo = {0};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "UnrealEngine25";
    appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
    appInfo.pEngineName = "UnrealEngine25";
    appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 1);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo instanceInfo = {0};
    uint32_t glfwExtensionCount = 0;
    const char **glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceInfo.enabledExtensionCount = glfwExtensionCount;
    instanceInfo.ppEnabledExtensionNames = glfwExtensions;
    instanceInfo.enabledLayerCount = 0;
    if (enableValidationLayers && check_validation_layer_support()) {
        instanceInfo.enabledLayerCount = validationLayerCount;
        instanceInfo.ppEnabledLayerNames = validationlayers;
    }

    VkResult result = vkCreateInstance(&instanceInfo, NULL, &instance);
    if (result != VK_SUCCESS) {
        printf("vkCreateInstance returned %d\n", VK_SUCCESS);
        return 1;
    }
    return 0;
}

typedef struct {
    int32_t graphicsFamily;
    int32_t presentaionFamily;
} QueueFamiliyIndices;
QueueFamiliyIndices find_queue_families(VkPhysicalDevice device) {
    QueueFamiliyIndices indices;
    indices.graphicsFamily = -1;
    indices.graphicsFamily = -1;

    uint32_t queueFamiliyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamiliyCount, NULL);
    VkQueueFamilyProperties queueFamilies[queueFamiliyCount];
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamiliyCount, queueFamilies);
    VkBool32 presentSupport = false;
    for (int i = 0; i < queueFamiliyCount; i++) {
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
        }
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
        if (presentSupport) {
            indices.presentaionFamily = i;
        }
    }
    return indices;
}

const char *deviceExtensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
const uint32_t requiredExtensionCount = 1;
bool check_device_extension_support(VkPhysicalDevice device) {
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, NULL, &extensionCount, NULL);
    VkExtensionProperties avaiableExtensions[extensionCount];

    vkEnumerateDeviceExtensionProperties(device, NULL, &extensionCount, avaiableExtensions);
    bool extensionsFound = true;

    for (int i = 0; i < requiredExtensionCount; i++) {
        extensionsFound = false;
        for (int j = 0; i < extensionCount; i++) {
            if (strcmp(deviceExtensions[i], avaiableExtensions[j].extensionName)) {
                extensionsFound = true;
                break;
            }
        }
    }
    return extensionsFound;
}

typedef struct {
    VkSurfaceCapabilitiesKHR capabilities;
    uint32_t formatCount;
    VkSurfaceFormatKHR *formats;
    uint32_t presentModeCount;
    VkPresentModeKHR *presentModes;
} SwapChainSupportDetails;

SwapChainSupportDetails query_swap_chain_support(VkPhysicalDevice device) {
    SwapChainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &details.formatCount, NULL);
    if (details.formatCount != 0) {
        details.formats = malloc(details.formatCount * sizeof(VkSurfaceFormatKHR));
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &details.formatCount, details.formats);
    }

    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &details.presentModeCount, NULL);
    if (details.presentModeCount != 0) {
        details.presentModes = malloc(details.presentModeCount * sizeof(VkPresentModeKHR));
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &details.presentModeCount, details.presentModes);
    }
    return details;
}
void free_SwapChainSupportDetails(SwapChainSupportDetails details) {
    free(details.presentModes);
    free(details.formats);
}
VkSurfaceFormatKHR choose_swap_surface_format(const SwapChainSupportDetails swapChainDetails) {
    for (int i = 0; i < swapChainDetails.formatCount; i++) {
        if (swapChainDetails.formats[i].format == VK_FORMAT_B8G8R8A8_SRGB &&
            swapChainDetails.formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return swapChainDetails.formats[i];
        }
    }
    return swapChainDetails.formats[0];
}
VkPresentModeKHR choost_swap_present_mode(const SwapChainSupportDetails swapChainDetails) {
    for (int i = 0; i < swapChainDetails.presentModeCount; i++) {
        if (swapChainDetails.presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
            return swapChainDetails.presentModes[i];
        }
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}
#define CLAMP_BETWEEN(val, min, max) (((val) < (min)) ? (min) : ((val) > (max)) ? (max) : (val))
VkExtent2D choose_swap_extent(const SwapChainSupportDetails swapChainDetails) {
    if (swapChainDetails.capabilities.currentExtent.width != UINT32_MAX) {
        return swapChainDetails.capabilities.currentExtent;
    } else {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        VkExtent2D actualExtent = {(uint32_t)width, (uint32_t)height};
        CLAMP_BETWEEN(actualExtent.width, swapChainDetails.capabilities.minImageExtent.width,
                      swapChainDetails.capabilities.maxImageExtent.width);
        CLAMP_BETWEEN(actualExtent.height, swapChainDetails.capabilities.minImageExtent.height,
                      swapChainDetails.capabilities.maxImageExtent.height);
        return actualExtent;
    }
}
uint8_t evalute_vulkan_device(VkPhysicalDevice device) {
    uint8_t score = 0;
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(device, &properties);
    VkPhysicalDeviceFeatures features;
    vkGetPhysicalDeviceFeatures(device, &features);
    printf("Evaluating %s \n", properties.deviceName);
    printf("DeviceType: %d ", properties.deviceType);
    switch (properties.deviceType) {
    case 0:
        printf("Other\n");
        break;
    case 1:
        printf("Ingegrated\n");
        score = 100;
        break;
    case 2:
        printf("Discrete\n");
        score = 200;
        break;
    case 3:
        printf("Virtual\n");
        score = 100;
        break;
    case 4:
        printf("CPU\n");
        score = 10;
        break;
    default:
        printf("unknown\n");
        score = 0;
        break;
    }
    if (!features.geometryShader)
        score = 0;

    QueueFamiliyIndices indices = find_queue_families(device);
    if (indices.graphicsFamily == -1) {
        score = 0;
        return score;
    }
    if (!check_device_extension_support(device)) {
        score = 0;
        return score;
    }
    SwapChainSupportDetails swapChainSupprt = query_swap_chain_support(device);
    if (swapChainSupprt.formatCount == 0 || swapChainSupprt.presentModeCount == 0) {
        score = 0;
        return score;
    }

    return score;
}
int pick_physical_vkdevice() {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, NULL);
    printf("Found %d Vulkan Devices\n", deviceCount);
    VkPhysicalDevice devices[deviceCount];
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices);
    uint8_t highScore = 0;
    int bestDevice = 0;
    for (int i = 0; i < deviceCount; i++) {
        uint8_t score = evalute_vulkan_device(devices[i]);
        if (highScore < score) {
            highScore = score;
            bestDevice = i;
        }
    }
    if (highScore == 0) {
        return -1;
    }
    physicalDevice = devices[bestDevice];
    return 0;
}

// TODO Could be made more flexible, instead of hardcoded FAMILYCOUNT
#define FAMILYCOUNT 2
int create_logical_device() {

    QueueFamiliyIndices indices = find_queue_families(physicalDevice);
    VkDeviceQueueCreateInfo queueCreateInfo[FAMILYCOUNT] = {};
    uint32_t uniqueQueues[FAMILYCOUNT] = {indices.graphicsFamily, indices.presentaionFamily};

    float queuePriority = 1.0f;
    for (int i = 0; i < FAMILYCOUNT; i++) {
        queueCreateInfo[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo[i].queueFamilyIndex = uniqueQueues[i];
        queueCreateInfo[i].queueCount = 1;
        queueCreateInfo[i].pQueuePriorities = &queuePriority;
    }
    VkPhysicalDeviceFeatures deviceFeatures = {0};
    VkDeviceCreateInfo deviceCreateInfo = {};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.pQueueCreateInfos = queueCreateInfo;
    deviceCreateInfo.queueCreateInfoCount = 2;
    deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
    deviceCreateInfo.enabledExtensionCount = requiredExtensionCount;
    deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions;
    if (vkCreateDevice(physicalDevice, &deviceCreateInfo, NULL, &device) != VK_SUCCESS) {
        printf("vkCreateDeviceInfo Failed line %d", __LINE__);
        return 1;
    }
    vkGetDeviceQueue(device, indices.graphicsFamily, 0, &graphicsQueue);
    vkGetDeviceQueue(device, indices.presentaionFamily, 0, &presentationQueue);
    return 0;
}

int create_surface() {
    if (glfwCreateWindowSurface(instance, window, NULL, &surface) != VK_SUCCESS) {
        printf("Failed to create window surface %d\n", __LINE__);
    }
}

int create_swap_chain() {
    SwapChainSupportDetails details = query_swap_chain_support(physicalDevice);
    VkSurfaceFormatKHR surfaceFormat = choose_swap_surface_format(details);
    VkPresentModeKHR presentMode = choost_swap_present_mode(details);
    VkExtent2D extent = choose_swap_extent(details);
    uint32_t imageCount = details.capabilities.minImageCount + 1;
    if (details.capabilities.maxImageCount > 0 && imageCount > details.capabilities.maxImageCount) {
        imageCount = details.capabilities.maxImageCount;
    }
    VkSwapchainCreateInfoKHR swapInfo = {0};
    swapInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapInfo.surface = surface;
    swapInfo.minImageCount = imageCount;
    swapInfo.imageFormat = surfaceFormat.format;
    swapInfo.imageColorSpace = surfaceFormat.colorSpace;
    swapInfo.imageExtent = extent;
    swapInfo.imageArrayLayers = 1;
    swapInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamiliyIndices indices = find_queue_families(physicalDevice);
    uint32_t queueFamiliesIndices[] = {indices.graphicsFamily, indices.presentaionFamily};

    if (indices.graphicsFamily != indices.presentaionFamily) {
        swapInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapInfo.queueFamilyIndexCount = 2;
        swapInfo.pQueueFamilyIndices = queueFamiliesIndices;
    } else {
        swapInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapInfo.queueFamilyIndexCount = 0;
        swapInfo.pQueueFamilyIndices = NULL;
    }
    swapInfo.preTransform = details.capabilities.currentTransform;
    swapInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapInfo.presentMode = presentMode;
    swapInfo.clipped = VK_TRUE;
    swapInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(device, &swapInfo, NULL, &swapChain) != VK_SUCCESS) {
        printf("vkCreateSwapChainKHR failed line %d\n", __LINE__);
        return 1;
    }

    vkGetSwapchainImagesKHR(device, swapChain, &swapChainImageCount, NULL);
    swapChainImages = malloc(swapChainImageCount * sizeof(VkImage));
    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages);

    swapChainImageFormat = surfaceFormat.format;
    swapChainExtent = extent;
    return 0;
}
int init_vulkan() {
    create_instance();
    create_surface();
    if (pick_physical_vkdevice() < 0) {
        printf("pick_physical_vkdevice Found zero compatible devies\n");
    }
    create_logical_device();
    create_swap_chain();

    return 0;
}

int deinit_vulkan() {
    vkDestroySwapchainKHR(device, swapChain, NULL);
    vkDestroyDevice(device, NULL);
    vkDestroySurfaceKHR(instance, surface, NULL);
    vkDestroyInstance(instance, NULL);

    return 0;
}

int main() {
    init_window();
    init_vulkan();

    // while (!glfwWindowShouldClose(window)) {
    //     glfwPollEvents();
    // }
    // do stuff
    //
    deinit_vulkan();
    deinit_window();
}
