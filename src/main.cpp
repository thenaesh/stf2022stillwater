#include <boilerplate.hpp>
#include <chrono>
#include <cstring>
#include <thread>
#include <window.hpp>
#include <vulkanstate.hpp>
#include <pipeline.hpp>

using namespace std::chrono_literals;
using namespace glm;


#include <shaderbytes.hpp>


struct PushConstants{
    vec2 time;
    ivec2 gridBounds;
    uvec2 jaga;  // 0 1 2 3
    uvec2 agaj;  // 1 2 3 0
    uvec2 gaja;  // 2 3 0 1
    uvec2 ajag;  // 3 0 1 2
    mat4 viewTransformMatrix;

    PushConstants(float t, ivec2 gridBounds, vec3 cameraPosition, vec3 cameraTarget, char const* input) {
        this->time = vec2{t, 0.0f};
        this->gridBounds = gridBounds;

        mat4 cameraMatrix = lookAt(
            cameraPosition,
            cameraTarget,
            vec3{0.0f, 0.0f, 1.0f}
        );
        mat4 perspectiveMatrix = perspective(
            -pi<float>() / 3,
            16.0f/9.0f,
            0.1f,
            1.0f);
        this->viewTransformMatrix = perspectiveMatrix * cameraMatrix;

        this->processInput(input);
    }

    void doRotatingCopy(char const* sb, char* db, size_t offset, size_t rotationOffset) {
        for (int i = 0; i < 4; i++) {
            int j = offset + ((rotationOffset + i) % 4);  // dest index
            int k = offset + i;  // src index
            db[j] = sb[k];
        }
    }

    void trollReverseEnd(char const* input, char* troll_buf) {
        char const* p = input + 6 + 32;
        while (!strncmp(p++, "\0", 1));
        for (int i = 0; i < 32; i++) {
            troll_buf[i] = *(p - i - 1);
        }
    }

    void trollRepeatWhatExists(char const* input, char* troll_buf, int end) {
        if (end < 1) {
            this->trollJaga(troll_buf);
            return;
        }
        for (int i = 0; i < 32; i++) {
            troll_buf[i] = input[(i % end)];
        }
    }

    void trollJaga(char* troll_buf) {
        troll_buf[0] = 'J';
        troll_buf[1] = 'A';
        troll_buf[2] = 'G';
        troll_buf[3] = 'A';

        for (int i = 4; i < 32; i++) {
            troll_buf[i] = troll_buf[i - 4];
        }
    }

    void processInput(char const* input) {
        char troll_buf[32];
        bool shouldTroll = false;
        
        if (strncmp(input, "STF22{", 6)) {
            shouldTroll = true;
            trollJaga(troll_buf);
        }

        if (strncmp(input + 6 + 32, "}", 1)) {
            shouldTroll = true;
            this->trollReverseEnd(input, troll_buf);
        }

        for (int i = 0; i < 32; i++) {
            if (input[6 + i] == '\0') {
                shouldTroll = true;
                this->trollRepeatWhatExists(input, troll_buf, i);
                break;
            }
        }

        uint32_t jaga_buf[8];  // to be copied over into the uvec2s

        char const* src_buf = nullptr;
        if (!shouldTroll) {
            src_buf = input + 6;
        } else {
            src_buf = troll_buf;
        }
        char* dest_buf = reinterpret_cast<char*>(jaga_buf);

        // populating jaga uvec2
        this->doRotatingCopy(src_buf, dest_buf, 0, 0);
        this->doRotatingCopy(src_buf, dest_buf, 4, 0);
        // populating agaj vec2
        this->doRotatingCopy(src_buf, dest_buf, 8, 1);
        this->doRotatingCopy(src_buf, dest_buf, 12, 1);
        // populating gaja uvec2
        this->doRotatingCopy(src_buf, dest_buf, 16, 2);
        this->doRotatingCopy(src_buf, dest_buf, 20, 2);
        // populating ajag uvec2
        this->doRotatingCopy(src_buf, dest_buf, 24, 3);
        this->doRotatingCopy(src_buf, dest_buf, 28, 3);

        this->jaga = uvec2{jaga_buf[0], jaga_buf[1]};
        this->agaj = uvec2{jaga_buf[2], jaga_buf[3]};
        this->gaja = uvec2{jaga_buf[4], jaga_buf[5]};
        this->ajag = uvec2{jaga_buf[6], jaga_buf[7]};
    }
};

struct Vertex {
    vec3 position;
    vec3 color;
    ivec2 coords;

    static constexpr unsigned int num_fields = 3;

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

    static constexpr VkVertexInputAttributeDescription getCoordsAttributeDescription() {
      return {
          .location = 2,
          .binding = 0,
          .format = VK_FORMAT_R32G32_SINT,
          .offset = offsetof(Vertex, coords),
      };
    }

    static constexpr array<VkVertexInputAttributeDescription, num_fields> getAttributeDescriptions() {
        return {
            getPositionAttributeDescription(),
            getColorAttributeDescription(),
            getCoordsAttributeDescription(),
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


vector<Vertex> generateVertexGrid(int num_rows, int num_cols, bool useWireFrame) {
    if (num_rows < 2 || num_cols < 2) {
        cerr << "ERROR 3" << endl;
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
            auto color = useWireFrame ? colors[(r + c) % 3]: vec3{0.02f, 0.46f, 0.52f};
            // get all triangles in window
            Vertex tl{
                .position = {
                    r * row_gap - 0.5f,
                    c * col_gap - 0.5f,
                    0.0f,
                },
                .color = color,
                .coords = {r, c},
            };
            Vertex tr{
                .position = {
                    r * row_gap - 0.5f,
                    (c + 1) * col_gap - 0.5f,
                    0.0f,
                },
                .color = color,
                .coords = {r, c + 1},
            };
            Vertex bl{
                .position = {
                    (r + 1) * row_gap - 0.5f,
                    c * col_gap - 0.5f,
                    0.0f,
                },
                .color = color,
                .coords = {r + 1, c},
            };
            Vertex br{
                .position = {
                    (r + 1) * row_gap - 0.5f,
                    (c + 1) * col_gap - 0.5f,
                    0.0f,
                },
                .color = color,
                .coords = {r + 1, c + 1},
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


float computeElapsedTime(chrono::steady_clock::time_point referenceTimePoint, float upperBound) {
    auto currentTimePoint = chrono::steady_clock::now();
    auto elapsedTime = currentTimePoint - referenceTimePoint;
    int elapsedMilliseconds = chrono::duration_cast<chrono::milliseconds>(elapsedTime).count();
    float elapsedSeconds = static_cast<float>(elapsedMilliseconds) / 1000.0f;

    return fmod(elapsedSeconds, upperBound);
}


int main(int argc, char** argv) {
    if (argc < 4 || argc > 5) {
        cerr << "Usage: ./stillwater <flag> <width in pixels> <height in pixels>" << endl;
        exit(1);
    }

    unsigned int width = atoi(argv[2]);
    unsigned int height = atoi(argv[3]);

    bool useWireFrame = argc == 5;

    WindowState window{"Still Water", width, height};
    VulkanState vkstate{window};

    vector<Shader> shaders;

    shaders.push_back(Shader{
        vkstate,
        VK_SHADER_STAGE_VERTEX_BIT,
        vertexShader,
        4640
    });
    shaders.push_back(Shader{
        vkstate,
        VK_SHADER_STAGE_FRAGMENT_BIT,
        fragmentShader,
        572
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

    auto referenceTimePoint = chrono::steady_clock::now();

    VertexBuffer<Vertex> vertexBuffer{vkstate, 10000000};

    while (window.isActive()) {

        // compute push constants based on perturbed variables
        auto pushConstants = PushConstants{
            computeElapsedTime(referenceTimePoint, 20.0f) - 10.0f,
            ivec2{512, 512},
            vec3{-0.5f, -0.5f, 0.1f},
            vec3{0.5f, 0.5f, 0.0f},
            argv[1],
        };

        vertexBuffer.setVertices(generateVertexGrid(
            512,
            512,
            useWireFrame));
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
