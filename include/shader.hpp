#ifndef _SHADER_H
#define _SHADER_H


#include <vulkanstate.hpp>


class Shader {
    bool hasMoved;

    VulkanState const& state;

    VkShaderStageFlagBits stage;

    vector<char> bytecode;
    VkShaderModule shaderModule;

public:

    Shader(VulkanState const& state, VkShaderStageFlagBits stage, string filename);
    Shader(VulkanState const& state, VkShaderStageFlagBits stage, unsigned char const* buf, size_t buflen);
    Shader(Shader&& o);
    virtual ~Shader();

private:

    void readFromFile(string filename);
    void readFromCharBuffer(unsigned char const* buf, size_t buflen);

    void createShaderModule();
    void destroyShaderModule();

    VkPipelineShaderStageCreateInfo generatePipelineStageCreateInfo() const;

public:

    static vector<VkPipelineShaderStageCreateInfo> computePipelineShaderStageCreateInfos(vector<Shader> const& shaders);
};


#endif
