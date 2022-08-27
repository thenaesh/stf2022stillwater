#ifndef _VULKANSTATE_H
#define _VULKANSTATE_H


#include <boilerplate.hpp>
#include <window.hpp>


typedef uint32_t QueueFamilyIndex;

typedef function<bool(VkPhysicalDevice)> PhysicalDeviceSuitabilityChecker;
typedef function<bool(VkPhysicalDevice, QueueFamilyIndex, VkQueueFamilyProperties const&)> QueueFamilySuitabilityChecker;

typedef tuple<VkPhysicalDevice, QueueFamilyIndex> QueueFamilyChoice;


struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    vector<VkSurfaceFormatKHR> formats;
    vector<VkPresentModeKHR> presentModes;
};

class VulkanState {
    WindowState& _window;

    VkInstance vkinstance;

    VkPhysicalDevice physicalDevice;
    VkDevice device;

public:
    uint32_t queueFamilyIndex;
    VkQueue queue;

    uint32_t presentQueueFamilyIndex;
    VkQueue presentQueue;

    VkSurfaceKHR surface;

    VkSwapchainKHR swapChain;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;

    vector<VkImage> swapChainImages;
    vector<VkImageView> imageViews;

    VkImage depthImage;
    VkDeviceMemory depthImageMemory;
    VkImageView depthImageView;

    VkCommandPool commandPool;

public:
    VulkanState(WindowState& window);
    virtual ~VulkanState();

    string getPhysicalDeviceProperties();

    operator VkDevice() const { return this->device; }
    operator VkPhysicalDevice() const { return this->physicalDevice; }
    operator VkCommandPool() const { return this->commandPool; }
    operator VkSwapchainKHR() const { return this->swapChain; }
    operator VkExtent2D() const { return this->swapChainExtent; }

    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

    void createImage(
        VkFormat format,
        VkImageTiling tiling,
        VkImageUsageFlags flags,
        VkMemoryPropertyFlags,
        VkImage& image,
        VkDeviceMemory&
        imageMemory);
    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags flags);

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
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(const vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
    void createSwapChain();
    void destroySwapChain();

    void createImageViews();
    void destroyImageViews();

    void createCommandPool();
    void destroyCommandPool();

    void createDepthBuffer();
    void destroyDepthBuffer();
};


#endif
