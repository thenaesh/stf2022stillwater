#include <cstring>
#include <algorithm>

#include <vulkanstate.hpp>

using namespace std;

#ifndef NDEBUG

const std::vector<const char *> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

bool checkValidationLayerSupport() {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char *layerName : validationLayers) {
      bool layerFound = false;

      for (const auto &layerProperties : availableLayers) {
        if (strcmp(layerName, layerProperties.layerName) == 0) {
          layerFound = true;
          break;
        }
      }

      if (!layerFound) {
        return false;
      }
    }

    return true;
}
#endif

const vector<string> REQUIRED_EXTENSIONS{
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
};

const auto REQUIRED_EXTENSIONS_RAW = map<string, const char*>(REQUIRED_EXTENSIONS, [](auto const& e){ return e.c_str(); });


VulkanState::VulkanState(WindowState& window) : _window{window} {
    this->initVulkanInstance();
    this->createSurface();
    this->selectPhysicalDevice();
    this->selectQueueFamily();
    this->createLogicalDeviceAndQueue();
    this->createSwapChain();
    this->createImageViews();
    this->createCommandPool();
}

VulkanState::~VulkanState() {
    this->destroyCommandPool();
    this->destroyImageViews();
    this->destroySwapChain();
    this->destroyLogicalDeviceAndQueue();
    this->destroySurface();
    this->destroyVulkanInstance();
}


string VulkanState::getPhysicalDeviceProperties() {

    VkPhysicalDeviceProperties pdp;
    vkGetPhysicalDeviceProperties(this->physicalDevice, &pdp);

    string props{""};

    props += pdp.deviceName;

    return props;
}


void VulkanState::initVulkanInstance() {
    VkApplicationInfo appInfo{
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = this->_window.title,
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName = "-",
        .engineVersion = VK_MAKE_VERSION(1, 0, 0),
        .apiVersion = VK_API_VERSION_1_3,
    };

    VkInstanceCreateInfo createInfo{
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &appInfo,
#ifdef NDEBUG
        .enabledLayerCount = 0,
#else
        .enabledLayerCount = static_cast<uint32_t>(validationLayers.size()),
        .ppEnabledLayerNames = validationLayers.data(),
#endif
                                  .enabledExtensionCount =
            this->_window.glfwExtensionCount,
        .ppEnabledExtensionNames = this->_window.glfwExtensions,
    };

#ifndef NDEBUG
    if (!checkValidationLayerSupport()) {
        cerr << "Validation layer errors" << endl;
        exit(1);
    }
#endif

    if (vkCreateInstance(&createInfo, nullptr, &this->vkinstance) != VK_SUCCESS) {
        cerr << "Error creating Vulkan instance" << endl;
        exit(1);
    }
}

void VulkanState::destroyVulkanInstance() {
    vkDestroyInstance(this->vkinstance, nullptr);
}


void VulkanState::createSurface() {
    if (glfwCreateWindowSurface(
        this->vkinstance,
        this->_window,
        nullptr,
        &this->surface) != VK_SUCCESS) {
    }
}

void VulkanState::destroySurface() {
    vkDestroySurfaceKHR(
        this->vkinstance,
        this->surface,
        nullptr
    );
}


VkPhysicalDevice VulkanState::findSuitablePhysicalDevice(PhysicalDeviceSuitabilityChecker f) {
    // enumerate physical devices

    uint32_t numDevices = 0;
    vkEnumeratePhysicalDevices(this->vkinstance, &numDevices, nullptr);

    vector<VkPhysicalDevice> physicalDevices{numDevices};
    vkEnumeratePhysicalDevices(this->vkinstance, &numDevices, physicalDevices.data());

    for (auto pd : physicalDevices) {
        if (f(pd)) {
            return pd;
        }
    }

    cerr << "No physical devices found" << endl;
    exit(1);
}

void VulkanState::selectPhysicalDevice() {
    this->physicalDevice = this->findSuitablePhysicalDevice([this](auto pd){
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(pd, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(pd, nullptr, &extensionCount, availableExtensions.data());

        auto availableExtensionNames = map<VkExtensionProperties, string>(availableExtensions, [](VkExtensionProperties const& x){ return string{x.extensionName}; });
        if (!contains(availableExtensionNames, REQUIRED_EXTENSIONS)) {
            return false;
        }

        VkPhysicalDeviceProperties deviceProperties;
        VkPhysicalDeviceFeatures deviceFeatures;

        vkGetPhysicalDeviceProperties(pd, &deviceProperties);
        vkGetPhysicalDeviceFeatures(pd, &deviceFeatures);

        bool isDiscreteGpu = deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
        bool isIntegratedGpu = deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU;
        bool isVirtualGpu = deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU;
        bool isCpu = deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_CPU;

        cout << deviceProperties.deviceType << endl;

        return (isDiscreteGpu || isIntegratedGpu || isVirtualGpu || isCpu) && deviceFeatures.geometryShader;
    });

    cout << "Physical Device " << this->getPhysicalDeviceProperties() << endl;
}


QueueFamilyChoice VulkanState::findSuitableQueueFamily(QueueFamilySuitabilityChecker f, VkPhysicalDevice pd) {
    // then enumerate queue families

    uint32_t numQueueFamilies;
    vkGetPhysicalDeviceQueueFamilyProperties(
        pd,
        &numQueueFamilies,
        nullptr
    );

    vector<VkQueueFamilyProperties> queueFamilies{numQueueFamilies};
    vkGetPhysicalDeviceQueueFamilyProperties(
        pd,
        &numQueueFamilies,
        queueFamilies.data()
    );

    // get all the queue families that fit the given criteria

    vector<QueueFamilyChoice> suitableQueueFamilyChoices;

    for (QueueFamilyIndex qfi = 0; qfi < queueFamilies.size(); qfi++) {
        auto& qf = queueFamilies[qfi];

        if (f(pd, qfi, qf)) {
            suitableQueueFamilyChoices.push_back(make_tuple(pd, qfi));
        }
    }

    if (suitableQueueFamilyChoices.empty()) {
        cerr << "No supported queue families found" << endl;
        exit(1);
    }

    return suitableQueueFamilyChoices[0];
}

void VulkanState::selectQueueFamily() {
    auto [a, graphics_qfi] = this->findSuitableQueueFamily([this](
            VkPhysicalDevice pd,
            QueueFamilyIndex qfi,
            VkQueueFamilyProperties const& qf){
        return (qf.queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0;
    }, this->physicalDevice);

    auto [b, present_qfi] = this->findSuitableQueueFamily([this](
            VkPhysicalDevice pd,
            QueueFamilyIndex qfi,
            VkQueueFamilyProperties const& qf){
        VkBool32 hasPresentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(
            pd,
            qfi,
            this->surface,
            &hasPresentSupport
        );
        return hasPresentSupport;
    }, this->physicalDevice);

    this->queueFamilyIndex = graphics_qfi;
    this->presentQueueFamilyIndex = present_qfi;
    
    cout << "Queue Family Index " << this->queueFamilyIndex << endl;
    cout << "Present Queue Family Index " << this->queueFamilyIndex << endl;
}


void VulkanState::createLogicalDeviceAndQueue() {
    float queuePriority = 1.0f;

    VkDeviceQueueCreateInfo queueCreateInfo{
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueFamilyIndex = this->queueFamilyIndex,
        .queueCount = 1,
        .pQueuePriorities = &queuePriority,
    };

    VkPhysicalDeviceFeatures deviceFeatures{};

    VkDeviceCreateInfo createInfo{
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = &queueCreateInfo,
        .enabledLayerCount = 0,
        .enabledExtensionCount = static_cast<uint32_t>(REQUIRED_EXTENSIONS.size()),
        .ppEnabledExtensionNames = REQUIRED_EXTENSIONS_RAW.data(),
        .pEnabledFeatures = &deviceFeatures,
    };

    if (vkCreateDevice(
        physicalDevice,
        &createInfo,
        nullptr,
        &device) != VK_SUCCESS) {
        cerr << "Unable to create device and queue" << endl;
        exit(1);
    }

    vkGetDeviceQueue(
        this->device,
        this->queueFamilyIndex,
        0,
        &this->queue
    );

    vkGetDeviceQueue(
        this->device,
        this->presentQueueFamilyIndex,
        0,
        &this->presentQueue
    );
}

void VulkanState::destroyLogicalDeviceAndQueue() {
    vkDestroyDevice(this->device, nullptr);
}


SwapChainSupportDetails VulkanState::querySwapChainSupport() {
    /*
    TODO: Cleanup!
    */
    SwapChainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(this->physicalDevice, this->surface, &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(this->physicalDevice, this->surface, &formatCount, nullptr);

    if (formatCount != 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(this->physicalDevice, this->surface, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(this->physicalDevice, this->surface, &presentModeCount, nullptr);

    if (presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(this->physicalDevice, this->surface, &presentModeCount, details.presentModes.data());
    }

    return details;
}

VkSurfaceFormatKHR VulkanState::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
    /*
    TODO: Cleanup!
    */
    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

VkPresentModeKHR VulkanState::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
    /*
    TODO: Cleanup!
    */
    for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return availablePresentMode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VulkanState::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
    /*
    TODO: Cleanup!
    */
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    } else {
        int width, height;
        glfwGetFramebufferSize(this->_window, &width, &height);

        VkExtent2D actualExtent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };

        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }
}

void VulkanState::createSwapChain() {
    SwapChainSupportDetails const swapChainSupport{this->querySwapChainSupport()};
    bool const isSwapChainAdequate{!(swapChainSupport.formats.empty() || swapChainSupport.presentModes.empty())};

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

    auto minImageCount = swapChainSupport.capabilities.minImageCount;
    auto maxImageCount = swapChainSupport.capabilities.maxImageCount;
    uint32_t imageCount = max(minImageCount + 1, min(minImageCount + 1, maxImageCount));

    bool isGraphicsAndPresentQueueSame = this->queueFamilyIndex == this->presentQueueFamilyIndex;
    uint32_t queueFamilyIndices[]{this->queueFamilyIndex, this->presentQueueFamilyIndex};

    VkSwapchainCreateInfoKHR createInfo{
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = surface,
        .minImageCount = imageCount,
        .imageFormat = surfaceFormat.format,
        .imageColorSpace = surfaceFormat.colorSpace,
        .imageExtent = extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .imageSharingMode = isGraphicsAndPresentQueueSame ? VK_SHARING_MODE_EXCLUSIVE : VK_SHARING_MODE_CONCURRENT,
        .queueFamilyIndexCount = isGraphicsAndPresentQueueSame ? 0u : 2u,
        .pQueueFamilyIndices = isGraphicsAndPresentQueueSame ? nullptr: queueFamilyIndices,
        .preTransform = swapChainSupport.capabilities.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = presentMode,
        .clipped = VK_TRUE,
        .oldSwapchain = VK_NULL_HANDLE,
    };

    if (vkCreateSwapchainKHR(this->device, &createInfo, nullptr, &this->swapChain) != VK_SUCCESS) {
        cerr << "Unable to create swap chain" << endl;
        exit(1);
    }

    vkGetSwapchainImagesKHR(this->device, this->swapChain, &imageCount, nullptr);
    this->swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(this->device, this->swapChain, &imageCount, this->swapChainImages.data());

    this->swapChainImageFormat = surfaceFormat.format;
    this->swapChainExtent = extent;
}

void VulkanState::destroySwapChain() {
    vkDestroySwapchainKHR(this->device, this->swapChain, nullptr);
}


void VulkanState::createImageViews() {
    this->imageViews.resize(this->swapChainImages.size());
    for (int i = 0; i < this->swapChainImages.size(); i++) {
        VkImageViewCreateInfo createInfo{
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = this->swapChainImages[i],
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = this->swapChainImageFormat,
            .components = VkComponentMapping{
                .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                .a = VK_COMPONENT_SWIZZLE_IDENTITY,
            },
            .subresourceRange = VkImageSubresourceRange{
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
        };

        if (vkCreateImageView(this->device, &createInfo, nullptr, &this->imageViews[i]) != VK_SUCCESS) {
            cerr << "Unable to create image view for image " << i << endl;
        }
    }
}

void VulkanState::destroyImageViews() {
    for (auto imageView : this->imageViews) {
        vkDestroyImageView(this->device, imageView, nullptr);
    }
}


void VulkanState::createCommandPool() {
    VkCommandPoolCreateInfo poolInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = this->queueFamilyIndex,
    };

    if (vkCreateCommandPool(
        this->device,
        &poolInfo,
        nullptr,
        &this->commandPool) != VK_SUCCESS) {
            cerr << "Failed to create command pool" << endl;
            exit(1);
    }
}

void VulkanState::destroyCommandPool() {
    vkDestroyCommandPool(this->device, this->commandPool, nullptr);
}
