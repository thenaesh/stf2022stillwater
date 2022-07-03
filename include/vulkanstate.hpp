#ifndef BOILERPLATE_H
#define BOILERPLATE_H
#include "vulkan/vulkan_core.h"
#include <boilerplate.hpp>
#endif

#ifndef WINDOW_H
#define WINDOW_H
#include <window.hpp>
#endif


class VulkanState {
    WindowState& _window;

    VkInstance vkinstance;

    VkPhysicalDevice physicalDevice;
    uint32_t queueFamilyIndex;

    VkDevice device;
    VkQueue queue;

    VkSurfaceKHR surface;

public:
    VulkanState(WindowState& window);
    virtual ~VulkanState();

    std::string getPhysicalDeviceProperties();

private:
    void initVulkanInstance();
    void destroyVulkanInstance();

    void createSurface();
    void destroySurface();

    void selectPhysicalDeviceAndQueueFamily();

    void createLogicalDeviceAndQueue();
    void destroyLogicalDeviceAndQueue();
};
