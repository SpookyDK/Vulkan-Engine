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

uint8_t evalute_vulkan_device(VkPhysicalDevice device) {
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
        break;
    case 2:
        printf("Discrete\n");
        break;
    case 3:
        printf("Virtual\n");
        break;
    case 4:
        printf("CPU\n");
        break;
    default:
        printf("unknown\n");
        break;
    }

    return 0;
}
int pick_physical_vkdevice() {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, NULL);
    printf("Found %d Vulkan Devices\n", deviceCount);
    VkPhysicalDevice devices[deviceCount];
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices);
    for (int i = 0; i < deviceCount; i++) {
        evalute_vulkan_device(devices[i]);
    }

    return 0;
}
int init_vulkan() {
    create_instance();
    pick_physical_vkdevice();

    return 0;
}

int deinit_vulkan() {
    vkDestroyInstance(instance, NULL);

    return 0;
}

int main() {
    init_window();
    init_vulkan();

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }
    // do stuff
    //
    deinit_vulkan();
    deinit_window();
}
