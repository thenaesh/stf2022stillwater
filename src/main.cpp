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


vector<Vertex> generateVertexGrid(int num_rows, int num_cols) {
    if (num_rows < 2 || num_cols < 2) {
        cerr << "Vertex grid must have at least 2 rows and 2 columns" << endl;
        exit(1);
    }

    auto row_gap = 1.0f / static_cast<float>(num_rows);
    auto col_gap = 1.0f / static_cast<float>(num_cols);

    vec3 colors[]{
        {1.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f},
        {0.0f, 0.0f, 1.0f},
    };

    vector<Vertex> triangleVertices;

    // use a sliding window of rectangles (4 vertices each)
    for (int r = 0; r < num_rows - 1; r++) {
        for (int c = 0; c < num_cols - 1; c++) {
            auto color = colors[(r + c) % 3];
            // get all triangles in window
            Vertex tl{
                .position = {r * row_gap - 0.5f, c * col_gap - 0.5f, 0.0f},
                .color = color
            };
            Vertex tr{
                .position = {r * row_gap - 0.5f, (c + 1) * col_gap - 0.5f, 0.0f},
                .color = color
            };
            Vertex bl{
                .position = {(r + 1) * row_gap - 0.5f, c * col_gap - 0.5f, 0.0f},
                .color = color
            };
            Vertex br{
                .position = {(r + 1) * row_gap - 0.5f, (c + 1) * col_gap - 0.5f, 0.0f},
                .color = color
            };
            // push first triangle
            triangleVertices.push_back(tl);
            triangleVertices.push_back(br);
            triangleVertices.push_back(bl);
            // push second triangle
            triangleVertices.push_back(tr);
            triangleVertices.push_back(br);
            triangleVertices.push_back(tl);
        }
    }

    return triangleVertices;
}


int main(int argc, char** argv) {
    WindowState window{"Still Water", 2560, 1440};
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

    VertexBuffer<Vertex> vertexBuffer{vkstate, 1000000};

    while (window.isActive()) {
        auto pushConstants = PushConstants{time, theta};
        time = (time >= 1.0f) ? -1.0f : time + 0.001f;
        theta = (theta >= two_pi<float>()) ? 0.0f : theta + 0.001f;

        auto t = abs(time);
        auto st = t * 0.2f;

        vertexBuffer.setVertices(generateVertexGrid(
            42,
            69));
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
