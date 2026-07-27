// This is my attempt at learning to draw a triangle in vulkan over the summer holidays
// Using the superior language of C over C++bloat
//
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vulkan/vulkan_core.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#define CGLM_HEADER_ONLY
#define CGLM_FORCE_RADIANS
#define CGLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define CGLM_FORCE_DEPTH_ZERO_TO_ONE // For Vulkan
#include "include/cglm/cglm.h"
#include "include/cglm/struct.h"
#include "include/my_vulkan/vulkan-device.h"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wall"
#pragma GCC diagnostic ignored "-Wextra"
#pragma GCC diagnostic ignored "-Wshadow"
#pragma GCC diagnostic ignored "-Wdouble-promotion"
#pragma GCC diagnostic ignored "-Wnull-dereference"
#pragma GCC diagnostic ignored "-Wanalyzer-possible-null-dereference"

#define STB_IMAGE_IMPLEMENTATION
#include "include/stb_image.h"

#define TINYOBJ_LOADER_C_IMPLEMENTATION
#include "./include/tiny_obj_c/tiny_obj_c.h"
#pragma GCC diagnostic pop
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 800
GLFWwindow *window;

VkInstance instance;

vkdevice_attributes_t mydevice;
// List of required Vulkan validation layers.
const char *validationlayers[] = {"VK_LAYER_KHRONOS_validation"};
const uint32_t validationLayerCount = 1;
// #ifdef NDEBUG
// const bool enableValidationLayers = false;
// #else
const bool enableValidationLayers = true;
// #endif
//
//
const char *ModelPath = "./models/viking_room_obj";
const char *TexturePath = "./textures/viking_room.png";

// VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
// VkDevice device = VK_NULL_HANDLE;
// VkQueue graphicsQueue = VK_NULL_HANDLE;
// VkQueue presentationQueue = VK_NULL_HANDLE;
VkSwapchainKHR swapChain;
VkFormat swapChainImageFormat;
VkExtent2D swapChainExtent;
VkImage *swapChainImages;
uint32_t swapChainImageCount = 0;
VkImageView *swapChainImageViews;
uint32_t swapChainImageViewCount = 0;
VkRenderPass renderpass;

VkDescriptorSetLayout descriptorSetLayout;
VkPipelineLayout pipelineLayout;
VkPipeline graphicsPipeline;
VkFramebuffer *swapChainFramebuffers;
uint32_t swapChainFrameBufferCount = 0;
VkCommandPool commandPool;

#define MAX_FRAMES_IN_FLIGHT 2
VkCommandBuffer commandBuffer[MAX_FRAMES_IN_FLIGHT];
VkSemaphore imageAvaiableSemaphores[MAX_FRAMES_IN_FLIGHT];
VkSemaphore renderFinishedSemaphores[MAX_FRAMES_IN_FLIGHT];
VkFence inFlightFence[MAX_FRAMES_IN_FLIGHT];
bool framebufferResized = false;

VkBuffer vertexBuffer;
VkDeviceMemory vertexBufferMemory;
VkBuffer indexBuffer;
VkDeviceMemory indexBufferMemory;

VkBuffer uniformBuffers[MAX_FRAMES_IN_FLIGHT];
VkDeviceMemory uniformBuffersMemory[MAX_FRAMES_IN_FLIGHT];
void *uniformBuffersMapped[MAX_FRAMES_IN_FLIGHT];

VkDescriptorPool descriptorPool;
VkDescriptorSet descriptorSets[MAX_FRAMES_IN_FLIGHT];

uint32_t textureMipLevels;
VkImage textureImage;
VkDeviceMemory textureImageMemory;
VkImageView textureImageView;
VkSampler textureSampler;

VkImage colorImage;
VkDeviceMemory colorImageMemory;
VkImageView colorImageView;

VkImage depthImage;
VkDeviceMemory depthImageMemory;
VkImageView depthImageView;

VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;
typedef struct {
    alignas(16) vec3s pos;
    alignas(16) vec3s color;
    alignas(16) vec2s texCoord;
} Vertex;
// Vertex vertices[] = {{.pos = {{-0.5f, -0.5f, 0.0f}}, .color = {{1.0f, 0.0f, 0.0f}}, .texCoord = {{0.0f, 0.0f}}},
//                      {.pos = {{0.5f, -0.5f, 0.0f}}, .color = {{0.0f, 1.0f, 0.0f}}, .texCoord = {{1.0f, 0.0f}}},
//                      {.pos = {{0.5f, 0.5f, 0.0f}}, .color = {{0.0f, 0.0f, 1.0f}}, .texCoord = {{1.0f, 1.0f}}},
//                      {.pos = {{-0.5f, 0.5f, 0.0f}}, .color = {{1.0f, 0.0f, 0.0f}}, .texCoord = {{0.0f, 1.0f}}},
//
//                      {.pos = {{-0.5f, -0.5f, -0.5f}}, .color = {{1.0f, 1.0f, 1.0f}}, .texCoord = {{0.0f, 0.0f}}},
//                      {.pos = {{0.5f, -0.5f, -0.5f}}, .color = {{0.0f, 1.0f, 0.0f}}, .texCoord = {{1.0f, 0.0f}}},
//                      {.pos = {{0.5f, 0.5f, -0.5f}}, .color = {{0.0f, 0.0f, 1.0f}}, .texCoord = {{1.0f, 1.0f}}},
//                      {.pos = {{-0.5f, 0.5f, -0.5f}}, .color = {{1.0f, 1.0f, 1.0f}}, .texCoord = {{0.0f, 1.0f}}}};
// const uint16_t indices[] = {0, 1, 2, 2, 3, 0, 4, 5, 6, 6, 7, 4};
Vertex *vertices;
uint32_t verticesCount;
uint32_t *indices;
uint32_t indicesCount;

VkVertexInputBindingDescription getBindingDescription(Vertex *_vertices,
                                                      uint32_t _verticesCount) { // TODO should delete these params / make it premade struct
    VkVertexInputBindingDescription bindingDescription = {};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(Vertex);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    return bindingDescription;
}
int getAttributeDescriptions(VkVertexInputAttributeDescription *pAttributeDescriptions, uint32_t *pAttributeDescriptionCount) {
    if (*pAttributeDescriptionCount == 0 || pAttributeDescriptions == NULL) {
        return 1;
    }
    pAttributeDescriptions[0].binding = 0;
    pAttributeDescriptions[0].location = 0;
    pAttributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    pAttributeDescriptions[0].offset = offsetof(Vertex, pos);

    pAttributeDescriptions[1].binding = 0;
    pAttributeDescriptions[1].location = 1;
    pAttributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    pAttributeDescriptions[1].offset = offsetof(Vertex, color);

    pAttributeDescriptions[2].binding = 0;
    pAttributeDescriptions[2].location = 2;
    pAttributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
    pAttributeDescriptions[2].offset = offsetof(Vertex, texCoord);

    return 0;
}

typedef struct {
    mat4 model;
    mat4 view;
    mat4 proj;
} UniformBufferObject;

void create_descriptor_set_layout() {
    VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
    samplerLayoutBinding.binding = 1;
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.pImmutableSamplers = NULL;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    VkDescriptorSetLayoutBinding uboLayoutBinding = {};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    uboLayoutBinding.pImmutableSamplers = NULL;
    VkDescriptorSetLayoutBinding bindings[] = {uboLayoutBinding, samplerLayoutBinding};
    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = sizeof(bindings) / sizeof(bindings[0]);
    // layoutInfo.bindingCount = 2;
    layoutInfo.pBindings = bindings;

    if (vkCreateDescriptorSetLayout(mydevice.device, &layoutInfo, NULL, &descriptorSetLayout) != VK_SUCCESS) {
        printf("vkCreateDescriptorSetLayout failed. c:%d\n", __LINE__);
    }
}
int load_shader_file(const char *filepath, char **out, uint64_t *out_len) {
    FILE *file = fopen(filepath, "rb");
    if (file == NULL) {
        printf("failed to open file %s c:%d\n", filepath, __LINE__);
        return 1;
    }
    fseek(file, 0, SEEK_END);
    uint64_t file_size = ftell(file);
    rewind(file);
    char *buffer = malloc(file_size);
    if (buffer == NULL) {
        printf("failed to malloc %" PRIu64 " bytes c:%d", file_size, __LINE__);
        fclose(file);
        return 1;
    }
    uint64_t read_size = fread(buffer, 1, file_size, file);
    if (read_size != file_size) {
        free(buffer);
        fclose(file);
        return 1;
    }
    *out = buffer;
    *out_len = file_size;
    fclose(file);

    return 0;
}

bool check_validation_layer_support() {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, NULL);
    VkLayerProperties avaiableLayers[layerCount];
    vkEnumerateInstanceLayerProperties(&layerCount, avaiableLayers);

    printf("Layercount is = %d", layerCount);
    for (uint32_t i = 0; i < validationLayerCount; i++) {
        bool layerFound = false;
        for (uint32_t j = 0; j < layerCount; j++) {
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
VkSampleCountFlagBits get_max_usable_msaa_sample_count() {
    VkSampleCountFlags counts =
        mydevice.properties.limits.framebufferColorSampleCounts & mydevice.properties.limits.framebufferDepthSampleCounts;
    if (counts & VK_SAMPLE_COUNT_64_BIT) {
        return VK_SAMPLE_COUNT_64_BIT;
    }
    if (counts & VK_SAMPLE_COUNT_32_BIT) {
        return VK_SAMPLE_COUNT_32_BIT;
    }
    if (counts & VK_SAMPLE_COUNT_16_BIT) {
        return VK_SAMPLE_COUNT_16_BIT;
    }
    if (counts & VK_SAMPLE_COUNT_8_BIT) {
        return VK_SAMPLE_COUNT_8_BIT;
    }
    if (counts & VK_SAMPLE_COUNT_4_BIT) {
        return VK_SAMPLE_COUNT_4_BIT;
    }
    if (counts & VK_SAMPLE_COUNT_2_BIT) {
        return VK_SAMPLE_COUNT_2_BIT;
    }

    return VK_SAMPLE_COUNT_1_BIT;
}
static void framebufferResizedCallback(GLFWwindow *_window, int width, int height) { framebufferResized = true; }
int init_window() {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "UnrealEngine25", NULL, NULL);
    glfwSetFramebufferSizeCallback(window, framebufferResizedCallback);
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
    instanceInfo.pApplicationInfo = &appInfo;
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
void create_descriptor_pool() {
    VkDescriptorPoolSize poolSizes[2] = {};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = MAX_FRAMES_IN_FLIGHT;

    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = MAX_FRAMES_IN_FLIGHT;
    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = sizeof(poolSizes) / sizeof(poolSizes[0]);
    poolInfo.pPoolSizes = poolSizes;
    poolInfo.maxSets = MAX_FRAMES_IN_FLIGHT;
    if (vkCreateDescriptorPool(mydevice.device, &poolInfo, NULL, &descriptorPool) != VK_SUCCESS) {
        printf("vkCreateDescriptorPool failed c:%d\n", __LINE__);
    }
}
void create_descriptor_sets() {
    VkDescriptorSetLayout layouts[MAX_FRAMES_IN_FLIGHT] = {descriptorSetLayout, descriptorSetLayout};
    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = MAX_FRAMES_IN_FLIGHT;
    allocInfo.pSetLayouts = layouts;
    if (vkAllocateDescriptorSets(mydevice.device, &allocInfo, descriptorSets) != VK_SUCCESS) {
        printf("vkAllocateDescriptorSets failed. c:%d\n", __LINE__);
        return;
    }
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        VkDescriptorBufferInfo bufferInfo = {};
        bufferInfo.buffer = uniformBuffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformBufferObject);
        VkDescriptorImageInfo imageInfo = {};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = textureImageView;
        imageInfo.sampler = textureSampler;

        VkWriteDescriptorSet descriptorWrites[2] = {};
        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = descriptorSets[i];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &bufferInfo;
        descriptorWrites[0].pImageInfo = NULL;
        descriptorWrites[0].pTexelBufferView = NULL;

        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = descriptorSets[i];
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pBufferInfo = NULL;
        descriptorWrites[1].pImageInfo = &imageInfo;
        vkUpdateDescriptorSets(mydevice.device, 2, descriptorWrites, 0, NULL);
    }
}

// NOTE: moved to vulkan-device.h
// typedef struct {
//     int32_t graphicsFamily;
//     int32_t presentaionFamily;
// } QueueFamiliyIndices;
// QueueFamiliyIndices find_queue_families(VkPhysicalDevice _device) {
//     QueueFamiliyIndices queueIndices;
//     queueIndices.graphicsFamily = -1;
//     queueIndices.presentaionFamily = -1;
//
//     uint32_t queueFamiliyCount = 0;
//     vkGetPhysicalDeviceQueueFamilyProperties(_device, &queueFamiliyCount, NULL);
//     VkQueueFamilyProperties queueFamilies[queueFamiliyCount];
//     vkGetPhysicalDeviceQueueFamilyProperties(_device, &queueFamiliyCount, queueFamilies);
//     VkBool32 presentSupport = false;
//     for (uint32_t i = 0; i < queueFamiliyCount; i++) {
//         if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
//             queueIndices.graphicsFamily = i;
//         }
//         vkGetPhysicalDeviceSurfaceSupportKHR(_device, i, surface, &presentSupport);
//         if (presentSupport) {
//             queueIndices.presentaionFamily = i;
//         }
//     }
//     return queueIndices;
// }

// NOTE: moved to vulkan-device.h
//  const char *deviceExtensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
//  const uint32_t requiredExtensionCount = 1;
//  bool check_device_extension_support(VkPhysicalDevice _device) {
//      uint32_t extensionCount;
//      vkEnumerateDeviceExtensionProperties(_device, NULL, &extensionCount, NULL);
//      VkExtensionProperties avaiableExtensions[extensionCount];
//
//      vkEnumerateDeviceExtensionProperties(_device, NULL, &extensionCount, avaiableExtensions);
//      bool extensionsFound = true;
//
//      for (uint32_t i = 0; i < requiredExtensionCount; i++) {
//          extensionsFound = false;
//          for (uint32_t j = 0; j < extensionCount; j++) {
//              if (!strcmp(deviceExtensions[i], avaiableExtensions[j].extensionName)) {
//                  extensionsFound = true;
//                  break;
//              }
//          }
//      }
//      return extensionsFound;
//  }

// NOTE: moved to vulkan-device.h
//  typedef struct {
//      VkSurfaceCapabilitiesKHR capabilities;
//      uint32_t formatCount;
//      VkSurfaceFormatKHR *formats;
//      uint32_t presentModeCount;
//      VkPresentModeKHR *presentModes;
//  } SwapChainSupportDetails;

// SwapChainSupportDetails create_SwapChainSupportDetails(VkPhysicalDevice _device) {
//     SwapChainSupportDetails details;
//
//     vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_device, surface, &details.capabilities);
//     vkGetPhysicalDeviceSurfaceFormatsKHR(_device, surface, &details.formatCount, NULL);
//     if (details.formatCount != 0) {
//         details.formats = malloc(details.formatCount * sizeof(VkSurfaceFormatKHR));
//         vkGetPhysicalDeviceSurfaceFormatsKHR(_device, surface, &details.formatCount, details.formats);
//     }
//
//     vkGetPhysicalDeviceSurfacePresentModesKHR(_device, surface, &details.presentModeCount, NULL);
//     if (details.presentModeCount != 0) {
//         details.presentModes = malloc(details.presentModeCount * sizeof(VkPresentModeKHR));
//         vkGetPhysicalDeviceSurfacePresentModesKHR(_device, surface, &details.presentModeCount, details.presentModes);
//     }
//     return details;
// }
// void free_SwapChainSupportDetails(SwapChainSupportDetails details) {
//     free(details.presentModes);
//     free(details.formats);
// }
VkSurfaceFormatKHR choose_swap_surface_format(const SwapChainSupportDetails swapChainDetails) {
    for (uint32_t i = 0; i < swapChainDetails.formatCount; i++) {
        if (swapChainDetails.formats[i].format == VK_FORMAT_B8G8R8A8_SRGB &&
            swapChainDetails.formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return swapChainDetails.formats[i];
        }
    }
    return swapChainDetails.formats[0];
}
VkPresentModeKHR choost_swap_present_mode(const SwapChainSupportDetails swapChainDetails) {
    for (uint32_t i = 0; i < swapChainDetails.presentModeCount; i++) {
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
        actualExtent.width = CLAMP_BETWEEN(actualExtent.width, swapChainDetails.capabilities.minImageExtent.width,
                                           swapChainDetails.capabilities.maxImageExtent.width);
        actualExtent.height = CLAMP_BETWEEN(actualExtent.height, swapChainDetails.capabilities.minImageExtent.height,
                                            swapChainDetails.capabilities.maxImageExtent.height);
        return actualExtent;
    }
}
// NOTE: moved to vulkan-device.h
// uint8_t evalute_vulkan_device(VkPhysicalDevice _device) {
//     uint8_t score = 0;
//     VkPhysicalDeviceProperties properties;
//     vkGetPhysicalDeviceProperties(_device, &properties);
//     VkPhysicalDeviceFeatures features;
//     vkGetPhysicalDeviceFeatures(_device, &features);
//     printf("Evaluating %s \n", properties.deviceName);
//     printf("DeviceType: %d ", properties.deviceType);
//     switch (properties.deviceType) {
//     case 0:
//         printf("Other\n");
//         break;
//     case 1:
//         printf("Ingegrated\n");
//         score = 100;
//         break;
//     case 2:
//         printf("Discrete\n");
//         score = 200;
//         break;
//     case 3:
//         printf("Virtual\n");
//         score = 100;
//         break;
//     case 4:
//         printf("CPU\n");
//         score = 10;
//         break;
//     default:
//         printf("unknown\n");
//         score = 0;
//         break;
//     }
//     if (!features.geometryShader)
//         score = 0;
//
//     QueueFamiliyIndices queueIndices = find_queue_families(_device);
//     if (queueIndices.graphicsFamily == -1) {
//         score = 0;
//         return score;
//     }
//     if (!check_device_extension_support(_device)) {
//         score = 0;
//         return score;
//     }
//     SwapChainSupportDetails swapChainSupport = create_SwapChainSupportDetails(_device);
//     if (swapChainSupport.formatCount == 0 || swapChainSupport.presentModeCount == 0) {
//         score = 0;
//         return score;
//     }
//     if (features.samplerAnisotropy != VK_TRUE) {
//         score = 0;
//         return score;
//     }
//     free_SwapChainSupportDetails(swapChainSupport);
//
//     return score;
// }
int pick_physical_device() {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, NULL);
    printf("Found %d Vulkan Devices\n", deviceCount);
    VkPhysicalDevice devices[deviceCount];
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices);
    uint8_t highScore = 0;
    int bestDevice = 0;
    vkdevice_attributes_t tempDeviceAttributes = {0};
    tempDeviceAttributes.surface = mydevice.surface;
    for (uint32_t i = 0; i < deviceCount; i++) {
        tempDeviceAttributes.physicalDevice = devices[i];
        evaluate_vulkan_device(&tempDeviceAttributes);
        if (highScore < tempDeviceAttributes.score) {
            highScore = tempDeviceAttributes.score;
            bestDevice = i;
        }
    }
    if (highScore == 0) {
        return -1;
    }

    printf("best device is %d %d\n", bestDevice, highScore);
    tempDeviceAttributes.physicalDevice = devices[bestDevice];
    mydevice = tempDeviceAttributes;
    msaaSamples = get_max_usable_msaa_sample_count();
    create_SwapChainSupportDetails(&mydevice);
    printf("MSAA = %dx\n", msaaSamples);
    return 0;
}

// TODO Could be made more flexible, instead of hardcoded FAMILYCOUNT
#define FAMILYCOUNT 2
// int create_logical_device() {
//     QueueFamiliyIndices queueIndices = find_queue_families(physicalDevice);
//     VkDeviceQueueCreateInfo queueCreateInfo[FAMILYCOUNT] = {};
//     uint32_t uniqueQueues[FAMILYCOUNT] = {queueIndices.graphicsFamily, queueIndices.presentaionFamily};
//
//     float queuePriority = 1.0f;
//     for (int i = 0; i < FAMILYCOUNT; i++) {
//         queueCreateInfo[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
//         queueCreateInfo[i].queueFamilyIndex = uniqueQueues[i];
//         queueCreateInfo[i].queueCount = 1;
//         queueCreateInfo[i].pQueuePriorities = &queuePriority;
//     }
//     VkPhysicalDeviceFeatures deviceFeatures = {0};
//     deviceFeatures.samplerAnisotropy = VK_TRUE;
//     deviceFeatures.sampleRateShading = VK_TRUE;
//     VkDeviceCreateInfo deviceCreateInfo = {};
//     deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
//     deviceCreateInfo.pQueueCreateInfos = queueCreateInfo;
//     deviceCreateInfo.queueCreateInfoCount = 2;
//     deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
//     deviceCreateInfo.enabledExtensionCount = requiredExtensionCount;
//     deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions;
//     if (vkCreateDevice(physicalDevice, &deviceCreateInfo, NULL, &device) != VK_SUCCESS) {
//         printf("vkCreateDeviceInfo Failed line %d", __LINE__);
//         return 1;
//     }
//     vkGetDeviceQueue(device, queueIndices.graphicsFamily, 0, &graphicsQueue);
//     vkGetDeviceQueue(device, queueIndices.presentaionFamily, 0, &presentationQueue);
//     return 0;
// }

int create_surface(VkSurfaceKHR *surface) {
    if (glfwCreateWindowSurface(instance, window, NULL, surface) != VK_SUCCESS) {
        printf("Failed to create window surface %d\n", __LINE__);
    }
    return 0;
}

int create_swap_chain(vkdevice_attributes_t *device_attributes) {
    free_Swap_chain_support_details(device_attributes->swapChainSupportDetails);
    create_SwapChainSupportDetails(device_attributes);
    VkSurfaceFormatKHR surfaceFormat = choose_swap_surface_format(device_attributes->swapChainSupportDetails);
    VkPresentModeKHR presentMode = choost_swap_present_mode(device_attributes->swapChainSupportDetails);
    VkExtent2D extent = choose_swap_extent(device_attributes->swapChainSupportDetails);
    uint32_t imageCount = device_attributes->swapChainSupportDetails.capabilities.minImageCount + 1;
    if (device_attributes->swapChainSupportDetails.capabilities.maxImageCount > 0 &&
        imageCount > device_attributes->swapChainSupportDetails.capabilities.maxImageCount) {
        imageCount = device_attributes->swapChainSupportDetails.capabilities.maxImageCount;
    }
    VkSwapchainCreateInfoKHR swapInfo = {0};
    swapInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapInfo.surface = mydevice.surface;
    swapInfo.minImageCount = imageCount;
    swapInfo.imageFormat = surfaceFormat.format;
    swapInfo.imageColorSpace = surfaceFormat.colorSpace;
    swapInfo.imageExtent = extent;
    swapInfo.imageArrayLayers = 1;
    swapInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    uint32_t queueFamiliesIndices[] = {device_attributes->queueFamilyIndices.graphicsFamily,
                                       device_attributes->queueFamilyIndices.presentaionFamily};

    if (device_attributes->queueFamilyIndices.graphicsFamily != device_attributes->queueFamilyIndices.presentaionFamily) {
        swapInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapInfo.queueFamilyIndexCount = 2;
        swapInfo.pQueueFamilyIndices = queueFamiliesIndices;
    } else {
        swapInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapInfo.queueFamilyIndexCount = 0;
        swapInfo.pQueueFamilyIndices = NULL;
    }
    swapInfo.preTransform = device_attributes->swapChainSupportDetails.capabilities.currentTransform;
    swapInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapInfo.presentMode = presentMode;
    swapInfo.clipped = VK_TRUE;
    swapInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(mydevice.device, &swapInfo, NULL, &swapChain) != VK_SUCCESS) {
        printf("vkCreateSwapChainKHR failed line %d\n", __LINE__);
        return 1;
    }

    vkGetSwapchainImagesKHR(mydevice.device, swapChain, &swapChainImageCount, NULL);
    swapChainImages = malloc(swapChainImageCount * sizeof(VkImage));
    vkGetSwapchainImagesKHR(mydevice.device, swapChain, &swapChainImageCount, swapChainImages);

    swapChainImageFormat = surfaceFormat.format;
    swapChainExtent = extent;
    // free_Swap_chain_support_details(device_attributes->swapChainSupportDetails);
    return 0;
}
VkImageView create_image_view(VkImage image, VkFormat format, uint32_t mipLevels, VkImageAspectFlags aspectFlags) {
    VkImageViewCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    createInfo.image = image;
    createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    createInfo.format = format;
    createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.subresourceRange.aspectMask = aspectFlags;
    createInfo.subresourceRange.baseMipLevel = 0;
    createInfo.subresourceRange.levelCount = mipLevels;
    createInfo.subresourceRange.baseArrayLayer = 0;
    createInfo.subresourceRange.layerCount = 1;
    VkImageView imageView;
    if (vkCreateImageView(mydevice.device, &createInfo, NULL, &imageView) != VK_SUCCESS) {
        printf("vkCreateImageView failed line: %d\n", __LINE__);
    }
    return imageView;
}
int create_swap_chain_image_views() {
    swapChainImageViewCount = swapChainImageCount;
    swapChainImageViews = malloc(swapChainImageViewCount * sizeof(VkImageView));
    for (uint32_t i = 0; i < swapChainImageViewCount; i++) {

        swapChainImageViews[i] = create_image_view(swapChainImages[i], swapChainImageFormat, 1, VK_IMAGE_ASPECT_COLOR_BIT);
        // VkImageViewCreateInfo createInfo = {};
        // createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        // createInfo.image = swapChainImages[i];
        // createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        // createInfo.format = swapChainImageFormat;
        // createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        // createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        // createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        // createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        // createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        // createInfo.subresourceRange.baseMipLevel = 0;
        // createInfo.subresourceRange.levelCount = 1;
        // createInfo.subresourceRange.baseArrayLayer = 0;
        // createInfo.subresourceRange.layerCount = 1;
        // if (vkCreateImageView(device, &createInfo, NULL, &swapChainImageViews[i]) != VK_SUCCESS) {
        //     printf("vkCreateImageView failed line: %d\n", __LINE__);
        // }
    }

    return 0;
}

VkShaderModule create_shader_module(const char *code, const uint64_t len) {
    VkShaderModuleCreateInfo createInfo = {0};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = len;
    createInfo.pCode = (const uint32_t *)code;
    VkShaderModule shaderModule;
    if (vkCreateShaderModule(mydevice.device, &createInfo, NULL, &shaderModule) != VK_SUCCESS) {
        printf("vkCreateShaderModule failed c:%d", __LINE__);
    }
    return shaderModule;
}
int create_graphichs_pipeline() {
    char *triangle_vert;
    uint64_t triangle_vert_len;
    load_shader_file("shaders/triangle/vert.spv", &triangle_vert, &triangle_vert_len);
    VkShaderModule vertShaderModule = create_shader_module(triangle_vert, triangle_vert_len);

    char *triangle_frag;
    uint64_t triangle_frag_len;
    load_shader_file("shaders/triangle/frag.spv", &triangle_frag, &triangle_frag_len);
    VkShaderModule fragShaderModule = create_shader_module(triangle_frag, triangle_frag_len);

    VkPipelineShaderStageCreateInfo vertShaderStageInfo = {0};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo = {0};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;

    VkPipelineShaderStageCreateInfo shaderstages[] = {vertShaderStageInfo, fragShaderStageInfo};
    // TODO maybe this not needed
    VkDynamicState dynamicStates[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    VkPipelineDynamicStateCreateInfo dynamicState = {};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = 2;
    dynamicState.pDynamicStates = dynamicStates;

    VkVertexInputBindingDescription bindingDescription = getBindingDescription(vertices, 3);
    uint32_t AttributeDescriptionCount = 3;
    VkVertexInputAttributeDescription AttributeDescriptions[AttributeDescriptionCount];
    getAttributeDescriptions(AttributeDescriptions, &AttributeDescriptionCount);

    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.vertexAttributeDescriptionCount = AttributeDescriptionCount;
    vertexInputInfo.pVertexAttributeDescriptions = AttributeDescriptions;

    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport = {0};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)swapChainExtent.width;
    viewport.height = (float)swapChainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor = {.offset = {.x = 0, .y = 0}, .extent = swapChainExtent};

    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;
    viewportState.pViewports = &viewport;

    VkPipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f;
    rasterizer.depthBiasClamp = 0.0f;
    rasterizer.depthBiasSlopeFactor = 0.0f;

    VkPipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_TRUE;
    multisampling.rasterizationSamples = msaaSamples;
    multisampling.minSampleShading = 0.2f;
    multisampling.pSampleMask = NULL;
    multisampling.alphaToCoverageEnable = VK_FALSE;
    multisampling.alphaToOneEnable = VK_FALSE;

    VkPipelineDepthStencilStateCreateInfo depthStencil = {};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.minDepthBounds = 0.0f;
    depthStencil.maxDepthBounds = 1.0f;
    depthStencil.stencilTestEnable = VK_FALSE;
    depthStencil.front = (VkStencilOpState){};
    depthStencil.back = (VkStencilOpState){};

    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo colorBlending = {};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f; // Optional
    colorBlending.blendConstants[1] = 0.0f; // Optional
    colorBlending.blendConstants[2] = 0.0f; // Optional
    colorBlending.blendConstants[3] = 0.0f; // Optional

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.pPushConstantRanges = NULL;

    if (vkCreatePipelineLayout(mydevice.device, &pipelineLayoutInfo, NULL, &pipelineLayout) != VK_SUCCESS) {
        printf("vkCreatePipelineLayout failed c:%d\n", __LINE__);
        vkDestroyShaderModule(mydevice.device, vertShaderModule, NULL);
        vkDestroyShaderModule(mydevice.device, fragShaderModule, NULL);
        free(triangle_vert);
        free(triangle_frag);
        return 1;
    }

    VkGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderstages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.renderPass = renderpass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex = -1;
    if (vkCreateGraphicsPipelines(mydevice.device, VK_NULL_HANDLE, 1, &pipelineInfo, NULL, &graphicsPipeline) != VK_SUCCESS) {
        printf("vkCreateGraphicsPipelines failed: c:%d\n", __LINE__);
        vkDestroyShaderModule(mydevice.device, vertShaderModule, NULL);
        vkDestroyShaderModule(mydevice.device, fragShaderModule, NULL);
        free(triangle_vert);
        free(triangle_frag);
        return 1;
    }
    vkDestroyShaderModule(mydevice.device, vertShaderModule, NULL);
    vkDestroyShaderModule(mydevice.device, fragShaderModule, NULL);
    free(triangle_vert);
    free(triangle_frag);

    return 0;
}
// TODO: move all the buffers into a seperate memory structure
int create_frame_buffers() {
    swapChainFrameBufferCount = swapChainImageViewCount;
    swapChainFramebuffers = malloc(swapChainFrameBufferCount * sizeof(VkFramebuffer));

    for (uint32_t i = 0; i < swapChainImageViewCount; i++) {
        VkImageView attachments[] = {colorImageView, depthImageView, swapChainImageViews[i]};

        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderpass;
        framebufferInfo.attachmentCount = 3;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = swapChainExtent.width;
        framebufferInfo.height = swapChainExtent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(mydevice.device, &framebufferInfo, NULL, &swapChainFramebuffers[i]) != VK_SUCCESS) {
            printf("vkCreateFrameBuffer failed idx %d, c:%d\n", i, __LINE__);
            return 1;
        }
    }

    return 0;
}

uint32_t currentFrame = 0;
void record_command_buffer(VkCommandBuffer _commandBuffer, uint32_t imageIndex) {
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0;
    beginInfo.pInheritanceInfo = NULL;

    if (vkBeginCommandBuffer(_commandBuffer, &beginInfo) != VK_SUCCESS) {
        printf("vkBeginCommandBuffer failed. c:%d\n", __LINE__);
        return;
    }
    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderpass;
    renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];
    renderPassInfo.renderArea.offset = (VkOffset2D){0, 0};
    renderPassInfo.renderArea.extent = swapChainExtent;

    VkClearValue clearValues[2] = {};
    clearValues[0].color = (VkClearColorValue){{0.0f, 0.0f, 0.0f, 1.0f}};
    clearValues[1].depthStencil = (VkClearDepthStencilValue){1.0f, 0};
    renderPassInfo.clearValueCount = 2;
    renderPassInfo.pClearValues = clearValues;

    vkCmdBeginRenderPass(_commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = swapChainExtent.width;
    viewport.height = swapChainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(_commandBuffer, 0, 1, &viewport);
    VkRect2D scissor = {};
    scissor.offset = (VkOffset2D){0, 0};
    scissor.extent = swapChainExtent;
    vkCmdSetScissor(_commandBuffer, 0, 1, &scissor);
    VkBuffer vertexBuffers[] = {vertexBuffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(_commandBuffer, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(_commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);
    // vkCmdDraw(commandBuffer, 3, 1, 0, 0);
    vkCmdBindDescriptorSets(_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[currentFrame], 0, NULL);
    vkCmdDrawIndexed(_commandBuffer, indicesCount, 4, 0, 0, 0);
    vkCmdEndRenderPass(_commandBuffer);
    if (vkEndCommandBuffer(_commandBuffer) != VK_SUCCESS) {
        printf("vkEndCommandBuffer failed, c:%d\n", __LINE__);
    }
    return;
}
int create_command_pool(vkdevice_attributes_t *device_attributes) {

    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = device_attributes->queueFamilyIndices.graphicsFamily;

    if (vkCreateCommandPool(mydevice.device, &poolInfo, NULL, &commandPool) != VK_SUCCESS) {
        printf("vkCreateCommandPool failed. c:%d\n", __LINE__);
        return 1;
    }

    return 0;
}
int create_command_buffer() {
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = MAX_FRAMES_IN_FLIGHT;

    if (vkAllocateCommandBuffers(mydevice.device, &allocInfo, commandBuffer) != VK_SUCCESS) {
        printf("vkAllocateCommandBuffers failed. c:%d\n", __LINE__);
        return 1;
    }
    return 0;
}
int create_sync_objects() {
    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    VkResult result = VK_SUCCESS;
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        result = vkCreateSemaphore(mydevice.device, &semaphoreInfo, NULL, &imageAvaiableSemaphores[i]);
        result = vkCreateSemaphore(mydevice.device, &semaphoreInfo, NULL, &renderFinishedSemaphores[i]);
        result = vkCreateFence(mydevice.device, &fenceInfo, NULL, &inFlightFence[i]);
    }
    if (result != VK_SUCCESS) {
        printf("Failed to create sync object c:%d\n", __LINE__);
        return 1;
    }
    return 0;
}
uint32_t find_memory_type(uint32_t typeFiler, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(mydevice.physicalDevice, &memProperties);
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if (typeFiler & (1 << i) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }
    printf("failed to find suitable memory type c:%d\n", __LINE__);
    return 0;
}
VkCommandBuffer begin_single_time_command() {
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;
    VkCommandBuffer singleCommandBuffer;
    vkAllocateCommandBuffers(mydevice.device, &allocInfo, &singleCommandBuffer);
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(singleCommandBuffer, &beginInfo);
    return singleCommandBuffer;
}
void end_single_time_commands(VkCommandBuffer _commandBuffer) {
    vkEndCommandBuffer(_commandBuffer);
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &_commandBuffer;
    vkQueueSubmit(mydevice.graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(mydevice.graphicsQueue);
    vkFreeCommandBuffers(mydevice.device, commandPool, 1, &_commandBuffer);
}
void copy_buffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
    VkCommandBuffer copyCommandBuffer = begin_single_time_command();
    VkBufferCopy copyRegion = {};
    copyRegion.size = size;
    vkCmdCopyBuffer(copyCommandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
    end_single_time_commands(copyCommandBuffer);
}
int create_buffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer *buffer,
                  VkDeviceMemory *buffermemory) {
    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(mydevice.device, &bufferInfo, NULL, buffer) != VK_SUCCESS) {
        printf("vkCreateBuffer failed size:%ld, c:%d\n", size, __LINE__);
        return 1;
    }
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(mydevice.device, *buffer, &memRequirements);
    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = find_memory_type(memRequirements.memoryTypeBits, properties);
    if (vkAllocateMemory(mydevice.device, &allocInfo, NULL, buffermemory) != VK_SUCCESS) {
        printf("vkAllocateMemory failed size:%ld, c:%d\n", memRequirements.size, __LINE__);
        return 1;
    }
    vkBindBufferMemory(mydevice.device, *buffer, *buffermemory, 0);
    return 0;
}

void copy_buffer_to_image(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
    VkCommandBuffer copyImageCommandBuffer = begin_single_time_command();
    VkBufferImageCopy region = {};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = (VkOffset3D){0, 0, 0};
    region.imageExtent = (VkExtent3D){width, height, 1};
    vkCmdCopyBufferToImage(copyImageCommandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
    end_single_time_commands(copyImageCommandBuffer);
}
void transitions_image_layout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels) {
    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;
    VkCommandBuffer transitionCommandBuffer = begin_single_time_command();
    VkImageMemoryBarrier barrier = {};
    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else {
        printf("unsupported layout transition c:%d\n", __LINE__);
    }
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = mipLevels;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    vkCmdPipelineBarrier(transitionCommandBuffer, sourceStage, destinationStage, 0, 0, NULL, 0, NULL, 1, &barrier);

    end_single_time_commands(transitionCommandBuffer);
}
int create_index_buffer() {
    VkDeviceSize bufferSize = indicesCount * sizeof(uint32_t);
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    create_buffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                  &stagingBuffer, &stagingBufferMemory);

    void *data;
    vkMapMemory(mydevice.device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, indices, bufferSize);
    vkUnmapMemory(mydevice.device, stagingBufferMemory);
    create_buffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                  &indexBuffer, &indexBufferMemory);
    copy_buffer(stagingBuffer, indexBuffer, bufferSize);
    vkDestroyBuffer(mydevice.device, stagingBuffer, NULL);
    vkFreeMemory(mydevice.device, stagingBufferMemory, NULL);

    return 0;
}
int create_vertex_buffer() {
    VkDeviceSize bufferSize = sizeof(Vertex) * verticesCount;
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    create_buffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                  &stagingBuffer, &stagingBufferMemory);

    void *data;
    vkMapMemory(mydevice.device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, vertices, bufferSize);
    vkUnmapMemory(mydevice.device, stagingBufferMemory);
    create_buffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                  &vertexBuffer, &vertexBufferMemory);
    copy_buffer(stagingBuffer, vertexBuffer, bufferSize);
    vkDestroyBuffer(mydevice.device, stagingBuffer, NULL);
    vkFreeMemory(mydevice.device, stagingBufferMemory, NULL);

    return 0;
}
int create_uniform_buffers() {
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        create_buffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &uniformBuffers[i],
                      &uniformBuffersMemory[i]);
        vkMapMemory(mydevice.device, uniformBuffersMemory[i], 0, bufferSize, 0, &uniformBuffersMapped[i]);
    }
    return 0;
}
int create_image(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format,
                 VkImageTiling tilling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage *image,
                 VkDeviceMemory *imageMemory) {

    VkImageCreateInfo imageInfo = {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = mipLevels;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tilling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = numSamples;
    imageInfo.flags = 0;
    if (vkCreateImage(mydevice.device, &imageInfo, NULL, image) != VK_SUCCESS) {
        printf("vkCreateImage failed. c:%d\n", __LINE__);
    }
    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(mydevice.device, *image, &memRequirements);
    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = find_memory_type(memRequirements.memoryTypeBits, properties);
    if (vkAllocateMemory(mydevice.device, &allocInfo, NULL, imageMemory) != VK_SUCCESS) {
        printf("vkAllocateMemory failed. c:%d\n", __LINE__);
        return 1;
    }
    vkBindImageMemory(mydevice.device, *image, *imageMemory, 0);
    return 0;
}

VkFormat find_support_format(const VkFormat *candidates, uint32_t candidatesCount, VkImageTiling tiling, VkFormatFeatureFlags features) {
    if (candidates != NULL && candidatesCount != 0 && candidatesCount != 0) {
    }
    for (uint32_t i = 0; i < candidatesCount; i++) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(mydevice.physicalDevice, candidates[i], &props);
        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
            return candidates[i];
        } else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
            return candidates[i];
        }
    }
    return candidates[0];
}
VkFormat find_depth_format() {
    return find_support_format((VkFormat[]){VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT}, 3,
                               VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}
bool has_stencil_component(VkFormat format) { return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT; }
int create_depth_resources() {
    VkFormat depthFormat = find_depth_format();
    create_image(swapChainExtent.width, swapChainExtent.height, 1, msaaSamples, depthFormat, VK_IMAGE_TILING_OPTIMAL,
                 VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &depthImage, &depthImageMemory);
    if (depthImage == NULL) {
        printf("is null\n");
    }
    depthImageView = create_image_view(depthImage, depthFormat, 1, VK_IMAGE_ASPECT_DEPTH_BIT);
    return 0;
}

int create_render_pass() {
    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format = swapChainImageFormat;
    colorAttachment.samples = msaaSamples;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription colorResolveAttachment = {};
    colorResolveAttachment.format = swapChainImageFormat;
    colorResolveAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorResolveAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorResolveAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorResolveAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorResolveAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorResolveAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorResolveAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorResolveRef = {};
    colorResolveRef.attachment = 2;
    colorResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription depthAttachment = {};
    depthAttachment.format = find_depth_format();
    depthAttachment.samples = msaaSamples;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef = {};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;
    subpass.pResolveAttachments = &colorResolveRef;

    VkAttachmentDescription attachments[3] = {colorAttachment, depthAttachment, colorResolveAttachment};
    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 3;
    renderPassInfo.pAttachments = attachments;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(mydevice.device, &renderPassInfo, NULL, &renderpass) != VK_SUCCESS) {
        printf("vkCreateRenderPass failed: c:%d\n", __LINE__);
        return 1;
    }
    return 0;
}
int generate_mipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels) {
    VkFormatProperties FormatProperties;
    vkGetPhysicalDeviceFormatProperties(mydevice.physicalDevice, imageFormat, &FormatProperties);
    if (!(FormatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
        printf("generate_mipmaps, linear blitting not supported c:%d\n", __LINE__);
    }
    VkCommandBuffer mipmapCommandBuffer = begin_single_time_command();
    VkImageMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.image = image;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.subresourceRange.levelCount = 1;
    int32_t mipWidth = texWidth;
    int32_t mipHeight = texHeight;
    for (uint32_t i = 1; i < mipLevels; i++) {
        barrier.subresourceRange.baseMipLevel = i - 1;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        vkCmdPipelineBarrier(mipmapCommandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 0, NULL, 1,
                             &barrier);
        VkImageBlit blit = {};
        blit.srcOffsets[0] = (VkOffset3D){0, 0, 0};
        blit.srcOffsets[1] = (VkOffset3D){mipWidth, mipHeight, 1};
        blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.srcSubresource.mipLevel = i - 1;
        blit.srcSubresource.baseArrayLayer = 0;
        blit.srcSubresource.layerCount = 1;
        blit.dstOffsets[0] = (VkOffset3D){0, 0, 0};
        blit.dstOffsets[1] = (VkOffset3D){mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1};
        blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.dstSubresource.mipLevel = i;
        blit.dstSubresource.baseArrayLayer = 0;
        blit.dstSubresource.layerCount = 1;
        vkCmdBlitImage(mipmapCommandBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
                       &blit, VK_FILTER_LINEAR);

        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        vkCmdPipelineBarrier(mipmapCommandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, NULL, 0,
                             NULL, 1, &barrier);
        if (mipWidth > 1)
            mipWidth /= 2;
        if (mipHeight > 1)
            mipHeight /= 2;
    }
    barrier.subresourceRange.baseMipLevel = mipLevels - 1;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    vkCmdPipelineBarrier(mipmapCommandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, NULL, 0, NULL, 1,
                         &barrier);

    end_single_time_commands(mipmapCommandBuffer);
    return 0;
}
int create_texture_image() {
    int texWidth, texHeight, texChannels;
    stbi_uc *pixels = stbi_load(TexturePath, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    VkDeviceSize imageSize = texWidth * texHeight * 4;
    textureMipLevels = (uint32_t)floor(log2(texWidth > texHeight ? texWidth : texHeight)) + 1;
    printf("create texture with %d mip levels c:%d\n", textureMipLevels, __LINE__);
    if (!pixels) {
        printf("Failed to load image %s, c:%d\n", TexturePath, __LINE__);
        return 1;
    }
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    create_buffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                  &stagingBuffer, &stagingBufferMemory);
    void *data;
    vkMapMemory(mydevice.device, stagingBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, pixels, imageSize);
    vkUnmapMemory(mydevice.device, stagingBufferMemory);
    stbi_image_free(pixels);

    create_image(texWidth, texHeight, textureMipLevels, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
                 VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &textureImage, &textureImageMemory);
    transitions_image_layout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                             textureMipLevels);
    copy_buffer_to_image(stagingBuffer, textureImage, texWidth, texHeight);
    // transitions_image_layout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
    //                          VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, textureMipLevels);

    generate_mipmaps(textureImage, VK_FORMAT_R8G8B8A8_SRGB, texWidth, texHeight, textureMipLevels);
    vkDestroyBuffer(mydevice.device, stagingBuffer, NULL);
    vkFreeMemory(mydevice.device, stagingBufferMemory, NULL);
    return 0;
}
int create_texture_image_view() {
    VkImageViewCreateInfo viewInfo = {};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = textureImage;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = textureMipLevels;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;
    if (vkCreateImageView(mydevice.device, &viewInfo, NULL, &textureImageView) != VK_SUCCESS) {
        printf("vkCreateImageView failed. c:%d\n", __LINE__);
    }
    return 0;
}
int create_texture_sampler() {
    VkSamplerCreateInfo samplerInfo = {};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_TRUE;
    VkPhysicalDeviceProperties properties = {};
    vkGetPhysicalDeviceProperties(mydevice.physicalDevice, &properties);
    samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
    samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 1.0f;
    samplerInfo.maxLod = VK_LOD_CLAMP_NONE;
    ;
    if (vkCreateSampler(mydevice.device, &samplerInfo, NULL, &textureSampler) != VK_SUCCESS) {
        printf("vkCreateSampler failed. c:%d\n", __LINE__);
        return 1;
    }
    return 0;
}
int load_model() {
    tobj_scene_f scene;
    tobj_load_config cfg = tobj_default_config();
    tobj_diag diag = {};
    if (tobj_load_obj_from_file_f(&scene, ModelPath, &cfg, &diag) != TOBJ_OK) {
        printf("failed to load model, %s c:%d\n ", diag.err, __LINE__);
        tobj_diag_free(&diag, NULL);
        return 1;
    }
    indicesCount = 0;
    for (uint32_t s = 0; s < scene.num_shapes; s++) {
        indicesCount += scene.shapes[s].mesh.num_indices;
    }
    vertices = malloc(sizeof(Vertex) * indicesCount);
    indices = malloc(sizeof(uint32_t) * indicesCount);
    verticesCount = indicesCount;

    if (!vertices || !indices) {
        printf("failed to allocate memory c:%d\n", __LINE__);
        tobj_diag_free(&diag, NULL);
        tobj_scene_free_f(&scene);
        return 1;
    }
    uint32_t currentCount = 0;
    for (uint32_t s = 0; s < scene.num_shapes; s++) {
        for (uint32_t i = 0; i < scene.shapes[s].mesh.num_indices; i++) {
            tobj_index idx = scene.shapes[s].mesh.indices[i];
            Vertex vertex = {};
            vertex.pos = (vec3s){{scene.attrib.vertices.ptr[3 * idx.vertex_index + 0], scene.attrib.vertices.ptr[3 * idx.vertex_index + 1],
                                  scene.attrib.vertices.ptr[3 * idx.vertex_index + 2]}};
            vertex.texCoord = (vec2s){
                {scene.attrib.texcoords.ptr[2 * idx.texcoord_index + 0], 1.0f - scene.attrib.texcoords.ptr[2 * idx.texcoord_index + 1]}};
            vertex.color = (vec3s){{1.0f, 1.0f, 1.0f}};
            vertices[currentCount] = vertex;
            indices[currentCount] = currentCount;
            currentCount++;
        }
    }
    tobj_scene_free_f(&scene);
    tobj_diag_free(&diag, NULL);
    return 0;
}

int create_color_resources() {
    VkFormat colorFormat = swapChainImageFormat;
    create_image(swapChainExtent.width, swapChainExtent.height, 1, msaaSamples, colorFormat, VK_IMAGE_TILING_OPTIMAL,
                 VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                 &colorImage, &colorImageMemory);
    colorImageView = create_image_view(colorImage, colorFormat, 1, VK_IMAGE_ASPECT_COLOR_BIT);
}
int init_vulkan() {
    const char *extensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    mydevice.pRequiredDeviceExtensions = extensions;
    mydevice.requiredExtensionCount = 1;
    create_instance();
    create_surface(&mydevice.surface);
    pick_physical_device();
    create_logical_device(&mydevice);
    create_swap_chain(&mydevice);
    create_swap_chain_image_views();
    create_render_pass();
    create_descriptor_set_layout();
    create_graphichs_pipeline();
    create_color_resources();
    create_depth_resources();
    create_frame_buffers();
    create_command_pool(&mydevice);
    create_texture_image();
    load_model();
    create_vertex_buffer();
    textureImageView = create_image_view(textureImage, VK_FORMAT_R8G8B8A8_SRGB, textureMipLevels, VK_IMAGE_ASPECT_COLOR_BIT);
    create_texture_sampler();
    create_index_buffer();
    create_uniform_buffers();
    create_descriptor_pool();
    create_descriptor_sets();
    create_command_buffer();
    create_sync_objects();

    return 0;
}

void cleanup_swap_chain() {
    for (uint32_t i = 0; i < swapChainFrameBufferCount; i++) {
        vkDestroyFramebuffer(mydevice.device, swapChainFramebuffers[i], NULL);
    }

    for (uint32_t i = 0; i < swapChainImageViewCount; i++) {
        vkDestroyImageView(mydevice.device, swapChainImageViews[i], NULL);
    }

    vkDestroyImageView(mydevice.device, colorImageView, NULL);
    vkDestroyImage(mydevice.device, colorImage, NULL);
    vkFreeMemory(mydevice.device, colorImageMemory, NULL);
    vkDestroyImageView(mydevice.device, depthImageView, NULL);
    vkDestroyImage(mydevice.device, depthImage, NULL);
    vkFreeMemory(mydevice.device, depthImageMemory, NULL);
    vkDestroySwapchainKHR(mydevice.device, swapChain, NULL);
    free(swapChainImageViews);
    free(swapChainImages);
    free(swapChainFramebuffers);
}
int deinit_vulkan() {
    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(mydevice.device, imageAvaiableSemaphores[i], NULL);
        vkDestroySemaphore(mydevice.device, renderFinishedSemaphores[i], NULL);
        vkDestroyFence(mydevice.device, inFlightFence[i], NULL);
    }
    vkDestroyCommandPool(mydevice.device, commandPool, NULL);

    cleanup_swap_chain();
    vkDestroySampler(mydevice.device, textureSampler, NULL);
    vkDestroyImageView(mydevice.device, textureImageView, NULL);
    vkDestroyImage(mydevice.device, textureImage, NULL);
    vkFreeMemory(mydevice.device, textureImageMemory, NULL);
    vkDestroyDescriptorPool(mydevice.device, descriptorPool, NULL);
    vkDestroyDescriptorSetLayout(mydevice.device, descriptorSetLayout, NULL);

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroyBuffer(mydevice.device, uniformBuffers[i], NULL);
        vkFreeMemory(mydevice.device, uniformBuffersMemory[i], NULL);
    }
    vkDestroyBuffer(mydevice.device, indexBuffer, NULL);
    vkFreeMemory(mydevice.device, indexBufferMemory, NULL);
    vkDestroyBuffer(mydevice.device, vertexBuffer, NULL);
    vkFreeMemory(mydevice.device, vertexBufferMemory, NULL);
    vkDestroyPipeline(mydevice.device, graphicsPipeline, NULL);
    vkDestroyPipelineLayout(mydevice.device, pipelineLayout, NULL);
    vkDestroyRenderPass(mydevice.device, renderpass, NULL);
    vkDestroyDevice(mydevice.device, NULL);
    vkDestroySurfaceKHR(instance, mydevice.surface, NULL);
    vkDestroyInstance(instance, NULL);

    return 0;
}

void recreate_swap_chain() {
    int width = 0, height = 0;
    glfwGetFramebufferSize(window, &width, &height);
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(window, &width, &height);
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(mydevice.device);
    cleanup_swap_chain();
    create_swap_chain(&mydevice);
    create_swap_chain_image_views();
    create_color_resources();
    create_depth_resources();
    create_frame_buffers();
}

void update_uniform_buffer(uint32_t currentImage) {
    UniformBufferObject ubo = {};
    glm_mat4_identity(ubo.model);
    float time = (float)glfwGetTime();
    glm_rotate(ubo.model, time * glm_rad(90.0f), (vec3){0.0f, 0.1f, 1.0f});
    glm_lookat((vec3){2.0f, 2.0f, 2.0f}, (vec3){0.0f, 0.0f, 0.0f}, (vec3){0.0f, 0.0f, 1.0f}, ubo.view);
    glm_perspective_rh_zo(glm_rad(45.0f), (float)swapChainExtent.width / (float)swapChainExtent.height, 0.1f, 10.0f, ubo.proj);
    ubo.proj[1][1] *= -1;
    memcpy(uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
}
void draw_frame() {
    vkWaitForFences(mydevice.device, 1, &inFlightFence[currentFrame], VK_TRUE, UINT64_MAX);
    uint32_t imageIndex;

    VkResult result =
        vkAcquireNextImageKHR(mydevice.device, swapChain, UINT64_MAX, imageAvaiableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreate_swap_chain();
        return;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        printf("Failed to acquire swap chain image. :c%d\n", __LINE__);
    }
    update_uniform_buffer(currentFrame);
    vkResetFences(mydevice.device, 1, &inFlightFence[currentFrame]);
    vkResetCommandBuffer(commandBuffer[currentFrame], 0);
    record_command_buffer(commandBuffer[currentFrame], imageIndex);
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    VkSemaphore waitSemaphores[] = {imageAvaiableSemaphores[currentFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer[currentFrame];

    VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;
    if (vkQueueSubmit(mydevice.graphicsQueue, 1, &submitInfo, inFlightFence[currentFrame]) != VK_SUCCESS) {
        printf("vkQueueSubmit failed. c:%d\n", __LINE__);
        return;
    }
    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {swapChain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;

    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = NULL;
    result = vkQueuePresentKHR(mydevice.presentationQueue, &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
        framebufferResized = false;
        recreate_swap_chain();
        return;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        printf("Filed to acquire swap chain image. :c%d\n", __LINE__);
    }
    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

int main() {
    init_window();
    init_vulkan();

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        draw_frame();
    }
    vkDeviceWaitIdle(mydevice.device);
    // do stuff
    //
    deinit_vulkan();
    deinit_window();
}
