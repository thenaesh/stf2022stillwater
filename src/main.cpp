#ifndef BOILERPLATE_H
#define BOILERPLATE_H
#include <boilerplate.hpp>
#endif

#ifndef WINDOW_H
#define WINDOW_H
#include <window.hpp>
#endif

#ifndef VULKANSTATE_H
#define VULKANSTATE_H
#include <vulkanstate.hpp>
#endif


int main(int argc, char** argv) {
    WindowState window{"Still Water", 1920, 1080};
    VulkanState vkstate{window};

    while (window.isActive()) {
        glfwPollEvents();
    }
}
