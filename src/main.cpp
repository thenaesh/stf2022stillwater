#include <boilerplate.hpp>
#include <window.hpp>
#include <vulkanstate.hpp>
#include <pipeline.hpp>

using namespace std;


int main(int argc, char** argv) {
    WindowState window{"Still Water", 1920, 1080};
    VulkanState vkstate{window};

    vector<Shader> shaders;

    shaders.push_back(Shader{
        vkstate,
        VK_SHADER_STAGE_VERTEX_BIT,
        "/home/thenaesh/Documents/still_water_vulkan/shaders/vert.spv"
    });
    shaders.push_back(Shader{
        vkstate,
        VK_SHADER_STAGE_FRAGMENT_BIT,
        "/home/thenaesh/Documents/still_water_vulkan/shaders/frag.spv"
    });

    Pipeline pipeline{vkstate, move(shaders)};

    while (window.isActive()) {
        glfwPollEvents();
    }
}
