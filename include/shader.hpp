#ifndef VULKANSTATE_H
#define VULKANSTATE_H
#include <vulkanstate.hpp>
#endif


class Shader {

    VulkanState const& state;

    VkShaderStageFlagBits stage;

    std::vector<char> bytecode;
    VkShaderModule shaderModule;

public:

    Shader(VulkanState const& state, VkShaderStageFlagBits stage, std::string filename);
    virtual ~Shader();

private:

    void readFromFile(std::string filename);

    void createShaderModule();
    void destroyShaderModule();

    VkPipelineShaderStageCreateInfo generatePipelineStageCreateInfo() const;

public:

    static std::vector<VkPipelineShaderStageCreateInfo> computePipelineShaderStageCreateInfos(std::vector<Shader> const& shaders);
};
