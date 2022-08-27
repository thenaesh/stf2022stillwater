#include <boilerplate.hpp>
#include <thread>
#include <window.hpp>
#include <vulkanstate.hpp>
#include <pipeline.hpp>

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

struct Vertex {
    vec3 position;
    vec3 color;

    static constexpr unsigned int num_fields = 2;

    static constexpr VkVertexInputAttributeDescription getPositionAttributeDescription() {
        return {
            .location = 0,
            .binding = 0,
            .format = VK_FORMAT_R32G32B32_SFLOAT,
            .offset = offsetof(Vertex, position),
        };
    }

    static constexpr VkVertexInputAttributeDescription getColorAttributeDescription() {
        return {
            .location = 1,
            .binding = 0,
            .format = VK_FORMAT_R32G32B32_SFLOAT,
            .offset = offsetof(Vertex, color),
        };
    }

    static constexpr array<VkVertexInputAttributeDescription, num_fields> getAttributeDescriptions() {
        return {
            getPositionAttributeDescription(),
            getColorAttributeDescription(),
        };
    }

    static constexpr VkVertexInputBindingDescription getBindingDescription() {
        return {
            .binding = 0,
            .stride = sizeof (Vertex),
            .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
        };
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

    auto vertexDescriptions = VertexDescriptions::create<Vertex>();

    Pipeline pipeline{
        vkstate,
        move(shaders),
        move(pushConstantRanges),
        move(vertexDescriptions)};

    float time = 0.0f;
    float theta = 0.0f;

    VertexBuffer<Vertex> vertexBuffer{vkstate, 6};

    while (window.isActive()) {
        auto pushConstants = PushConstants{time, theta};
        time = (time >= 1.0f) ? -1.0f : time + 0.001f;
        theta = (theta >= two_pi<float>()) ? 0.0f : theta + 0.001f;

        auto t = abs(time);
        auto st = t * 0.2f;

        vertexBuffer.setVertices({
            Vertex{.position = {-0.38f, -0.38f, 0.0f}, .color = {1.0f - t, t, 0.0f}},
            Vertex{.position = {0.38f, 0.38f, 0.0f}, .color = {0.0f, 1.0f - t, t}},
            Vertex{.position = {-0.38f - st, 0.38f + st, 0.0f}, .color = {t, 0.0f, 1.0f - t}},
            Vertex{.position = {-0.38f, -0.38f, 0.0f}, .color = {1.0f - t, t, 0.0f}},
            Vertex{.position = {0.38f + st, -0.38f - st, 0.0f}, .color = {1.0f - t, t, 0.0f}},
            Vertex{.position = {0.38f, 0.38f, 0.0f}, .color = {0.0f, 1.0f - t, t}},
        });
        vertexBuffer.syncWithGpuMemory();

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

            vkCmdBindPipeline(
                commandBuffer,
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                pipeline);

            VkBuffer vertexBuffers[] = {vertexBuffer};
            VkDeviceSize offsets[] = {0};
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

            vkCmdDraw(
                commandBuffer,
                vertexBuffer.numVertices(),
                1,
                0,
                0);
        });
        this_thread::sleep_for(1ms);
    }

    pipeline.waitIdle();
}
