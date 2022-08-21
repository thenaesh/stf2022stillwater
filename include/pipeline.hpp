#ifndef _PIPELINE_H
#define _PIPELINE_H


#include <vulkanstate.hpp>
#include <shader.hpp>


typedef std::function<void(VkCommandBuffer&, VkExtent2D const&)> CommandBufferRecorder;


class Pipeline {

    VulkanState const& state;

    std::vector<Shader> shaders;

    VkRenderPass renderPass;
    VkPipelineLayout pipelineLayout;
    VkPipeline pipeline;
    std::vector<VkPushConstantRange> pushConstantRanges;

    std::vector<VkFramebuffer> framebuffers;
    VkCommandBuffer commandBuffer;

    VkSemaphore imageAvailableSemaphore;
    VkSemaphore renderFinishedSemaphore;
    VkFence inFlightFence;

public:

    Pipeline(VulkanState const& state, std::vector<Shader> shaders, std::vector<VkPushConstantRange> pushConstantRanges);
    virtual ~Pipeline();

private:

    void createRenderPass();
    void destroyRenderPass();

    void createPipelineLayout();
    void destroyPipelineLayout();

    void createPipeline();
    void destroyPipeline();

    void createFramebuffers();
    void destroyFramebuffers();

    void createSyncPrimitives();
    void destroySyncPrimitives();

    void allocateCommandBuffer();

    void recordRenderPass(CommandBufferRecorder f, uint32_t imageIndex);

public:
    void render(CommandBufferRecorder f);
    void waitIdle();
    void pushConstant(size_t constantSize, void const* constant);
};


#endif
