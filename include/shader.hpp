#ifndef _SHADER_H
#define _SHADER_H


#include <vulkanstate.hpp>


class Shader {
    bool hasMoved;

    VulkanState const& state;

    VkShaderStageFlagBits stage;

    std::vector<char> bytecode;
    VkShaderModule shaderModule;

public:

    Shader(VulkanState const& state, VkShaderStageFlagBits stage, std::string filename);
    Shader(Shader&& o);
    virtual ~Shader();

private:

    void readFromFile(std::string filename);

    void createShaderModule();
    void destroyShaderModule();

    VkPipelineShaderStageCreateInfo generatePipelineStageCreateInfo() const;

public:

    static std::vector<VkPipelineShaderStageCreateInfo> computePipelineShaderStageCreateInfos(std::vector<Shader> const& shaders);
};


#endif
