#ifndef _VULKANSTATE_H
#define _VULKANSTATE_H


#include <boilerplate.hpp>
#include <window.hpp>


typedef uint32_t QueueFamilyIndex;

typedef std::function<bool(VkPhysicalDevice)> PhysicalDeviceSuitabilityChecker;
typedef std::function<bool(VkPhysicalDevice, QueueFamilyIndex, VkQueueFamilyProperties const&)> QueueFamilySuitabilityChecker;

typedef std::tuple<VkPhysicalDevice, QueueFamilyIndex> QueueFamilyChoice;


struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

class VulkanState {
    WindowState& _window;

    VkInstance vkinstance;

    VkPhysicalDevice physicalDevice;
    VkDevice device;

    uint32_t queueFamilyIndex;
    VkQueue queue;

    uint32_t presentQueueFamilyIndex;
    VkQueue presentQueue;

public:
    VkSurfaceKHR surface;

    VkSwapchainKHR swapChain;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    std::vector<VkImage> swapChainImages;

    std::vector<VkImageView> imageViews;

public:
    VulkanState(WindowState& window);
    virtual ~VulkanState();

    std::string getPhysicalDeviceProperties();

    operator VkDevice() const { return this->device; }
    operator VkPhysicalDevice() const { return this->physicalDevice; }

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

    SwapChainSupportDetails querySwapChainSupport();
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
    void createSwapChain();
    void destroySwapChain();

    void createImageViews();
    void destroyImageViews();
};


#endif
