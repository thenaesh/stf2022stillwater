#ifndef VULKANSTATE_H
#define VULKANSTATE_H
#include "vulkan/vulkan_core.h"
#include <vulkanstate.hpp>
#endif

using namespace std;


VulkanState::VulkanState(WindowState& window) : _window{window} {
    this->initVulkanInstance();
    this->createSurface();
    this->selectPhysicalDevice();
    this->selectQueueFamily();
    this->createLogicalDeviceAndQueue();
}

VulkanState::~VulkanState() {
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
        .enabledLayerCount = 0,
        .enabledExtensionCount = this->_window.glfwExtensionCount,
        .ppEnabledExtensionNames = this->_window.glfwExtensions,
    };

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
        VkPhysicalDeviceProperties deviceProperties;
        VkPhysicalDeviceFeatures deviceFeatures;

        vkGetPhysicalDeviceProperties(pd, &deviceProperties);
        vkGetPhysicalDeviceFeatures(pd, &deviceFeatures);

        return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
               deviceFeatures.geometryShader;
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
        .enabledExtensionCount = 0,
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
