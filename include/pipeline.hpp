#ifndef _PIPELINE_H
#define _PIPELINE_H


#include <vulkanstate.hpp>
#include <shader.hpp>


class Pipeline {

    VulkanState const& state;

    std::vector<Shader> shaders;

    VkRenderPass renderPass;
    VkPipelineLayout pipelineLayout;
    VkPipeline pipeline;

public:

    Pipeline(VulkanState const& state, std::vector<Shader> shaders);
    virtual ~Pipeline();

private:

    void createRenderPass();
    void destroyRenderPass();

    void createPipelineLayout();
    void destroyPipelineLayout();

    void createPipeline();
    void destroyPipeline();
};


#endif
