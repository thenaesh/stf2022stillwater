#include <boilerplate.hpp>
#include <thread>
#include <window.hpp>
#include <vulkanstate.hpp>
#include <pipeline.hpp>

using namespace std;
using namespace std::chrono_literals;
using namespace glm;


struct PushConstants{
    vec4 time;
    mat4 rotationMatrix;

    PushConstants(float t, float theta) {
        this->time = vec4{t, 0.0f, 0.0f, 0.0f};
        this->rotationMatrix = rotate(
            mat4{1.0f},
            theta,
            normalize(vec3{0.0f, 0.0f, 1.0f})
        );
    }
};


int main(int argc, char** argv) {
    WindowState window{"Still Water", 1024, 1024};
    VulkanState vkstate{window};

    vector<Shader> shaders;

    shaders.push_back(Shader{
        vkstate,
        VK_SHADER_STAGE_VERTEX_BIT,
        (argc > 1) ? argv[1] : "/home/thenaesh/Documents/still_water_vulkan/shaders/vert.spv"
    });
    shaders.push_back(Shader{
        vkstate,
        VK_SHADER_STAGE_FRAGMENT_BIT,
        (argc > 2) ? argv[2] : "/home/thenaesh/Documents/still_water_vulkan/shaders/frag.spv"
    });

    vector<VkPushConstantRange> pushConstantRanges;

    pushConstantRanges.push_back(VkPushConstantRange{
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
        .offset = 0,
        .size = sizeof (PushConstants),
    });

    Pipeline pipeline{
        vkstate,
        move(shaders),
        move(pushConstantRanges)};

    float time = 0.0f;
    float theta = 0.0f;

    while (window.isActive()) {
        auto pushConstants = PushConstants{time, theta};
        time = (time >= 1.0f) ? -1.0f : time + 0.001f;
        theta = (theta >= two_pi<float>()) ? 0.0f : theta + 0.001f;

        glfwPollEvents();
        pipeline.render([&](VkCommandBuffer& commandBuffer, VkExtent2D const& swapChainExtent) {
            VkViewport viewport{};
            viewport.x = 0.0f;
            viewport.y = 0.0f;
            viewport.width = static_cast<float>(swapChainExtent.width);
            viewport.height = static_cast<float>(swapChainExtent.height);
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;
            vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

            VkRect2D scissor{};
            scissor.offset = {0, 0};
            scissor.extent = swapChainExtent;
            vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

            pipeline.pushConstant(sizeof (PushConstants), &pushConstants);

            vkCmdDraw(commandBuffer, 3, 1, 0, 0);
        });
        this_thread::sleep_for(1ms);
    }

    pipeline.waitIdle();
}
