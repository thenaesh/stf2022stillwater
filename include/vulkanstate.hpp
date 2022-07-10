#ifndef BOILERPLATE_H
#define BOILERPLATE_H
#include "vulkan/vulkan_core.h"
#include <boilerplate.hpp>
#endif

#ifndef WINDOW_H
#define WINDOW_H
#include <window.hpp>
#endif


typedef uint32_t QueueFamilyIndex;

typedef std::function<bool(VkPhysicalDevice)> PhysicalDeviceSuitabilityChecker;
typedef std::function<bool(VkPhysicalDevice, QueueFamilyIndex, VkQueueFamilyProperties const&)> QueueFamilySuitabilityChecker;

typedef std::tuple<VkPhysicalDevice, QueueFamilyIndex> QueueFamilyChoice;


class VulkanState {
    WindowState& _window;

    VkInstance vkinstance;


    VkPhysicalDevice physicalDevice;
    VkDevice device;

    uint32_t queueFamilyIndex;
    VkQueue queue;

    uint32_t presentQueueFamilyIndex;
    VkQueue presentQueue;

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

    VkPhysicalDevice findSuitablePhysicalDevice(PhysicalDeviceSuitabilityChecker f);
    void selectPhysicalDevice();

    QueueFamilyChoice findSuitableQueueFamily(QueueFamilySuitabilityChecker f, VkPhysicalDevice pd);
    void selectQueueFamily();

    void createLogicalDeviceAndQueue();
    void destroyLogicalDeviceAndQueue();
};
