#ifndef VULKANSTATE_H
#define VULKANSTATE_H
#include "vulkan/vulkan_core.h"
#include <vulkanstate.hpp>
#endif

using namespace std;


typedef uint32_t QueueFamilyIndex;


VulkanState::VulkanState(WindowState& window) : _window{window} {
    this->initVulkanInstance();
    this->createSurface();
    this->selectPhysicalDeviceAndQueueFamily();
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


void VulkanState::selectPhysicalDeviceAndQueueFamily() {
    // enumerate physical devices

    uint32_t numDevices = 0;
    vkEnumeratePhysicalDevices(this->vkinstance, &numDevices, nullptr);

    if (numDevices == 0){
        cerr << "No physical devices found" << endl;
        exit(1);
    }

    vector<VkPhysicalDevice> physicalDevices{numDevices};
    vkEnumeratePhysicalDevices(this->vkinstance, &numDevices, physicalDevices.data());

    // then enumerate queue families

    vector<vector<VkQueueFamilyProperties>*> queueFamilies{numDevices};

    for (int pdi = 0; pdi < numDevices; pdi++) {
        uint32_t numQueueFamilies;
        vkGetPhysicalDeviceQueueFamilyProperties(
            physicalDevices[pdi],
            &numQueueFamilies,
            nullptr
        );

        queueFamilies[pdi] = new vector<VkQueueFamilyProperties>{numQueueFamilies};
        vkGetPhysicalDeviceQueueFamilyProperties(
            physicalDevices[pdi],
            &numQueueFamilies,
            queueFamilies[pdi]->data()
        );
    }

    vector<tuple<VkPhysicalDevice, QueueFamilyIndex>> suitableDevQfCombos;

    for (int pdi = 0; pdi < queueFamilies.size(); pdi++) {
        auto pd = physicalDevices[pdi];
        auto& qfs = *queueFamilies[pdi];

        for (QueueFamilyIndex qfi = 0; qfi < queueFamilies[pdi]->size(); qfi++) {
            auto& qf = qfs[qfi];

            VkBool32 hasPresentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(
                pd,
                qfi,
                this->surface,
                &hasPresentSupport
            );

            if ((qf.queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0 && hasPresentSupport) {
                suitableDevQfCombos.push_back(make_tuple(pd, qfi));
            }
        }
    }

    if (suitableDevQfCombos.empty()) {
        cerr << "No queue families supporting graphics found" << endl;
        exit(1);
    }

    auto [pd, qfi] = suitableDevQfCombos[0];

    this->physicalDevice = pd;
    this->queueFamilyIndex = qfi;

    cout << "Physical Device " << this->getPhysicalDeviceProperties() << endl;
    cout << "Queue Family Index " << this->queueFamilyIndex << endl;
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
}

void VulkanState::destroyLogicalDeviceAndQueue() {
    vkDestroyDevice(this->device, nullptr);
}
