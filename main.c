// This is my attempt at learning to draw a triangle in vulkan over the summer holidays
// Using the superior language of C over C++bloat
//
#include <stdbool.h>
#include <stdio.h>
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
} QueueFamiliyIndices;
QueueFamiliyIndices find_queue_families(VkPhysicalDevice device) {
    QueueFamiliyIndices indices;
    indices.graphicsFamily = -1;

    uint32_t queueFamiliyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamiliyCount, NULL);
    VkQueueFamilyProperties queueFamilies[queueFamiliyCount];
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamiliyCount, queueFamilies);
    for (int i = 0; i < queueFamiliyCount; i++) {
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
        }
    }
    return indices;
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

int create_logical_device() {

    QueueFamiliyIndices indices = find_queue_families(physicalDevice);
    VkDeviceQueueCreateInfo queueCreateInfo = {};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = indices.graphicsFamily;
    queueCreateInfo.queueCount = 1;
    return 0;
}
int init_vulkan() {
    create_instance();
    if (pick_physical_vkdevice() < 0) {
        printf("pick_physical_vkdevice Found zero compatible devies\n");
    }
    create_logical_device();

    return 0;
}

int deinit_vulkan() {
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
