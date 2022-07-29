#include <fstream>

#ifndef SHADER_H
#define SHADER_H
#include <shader.hpp>
#endif

using namespace std;


Shader::Shader(VulkanState const& state, VkShaderStageFlagBits stage, string filename): state{state}, stage{stage} {
    this->readFromFile(filename);
    this->createShaderModule();
}

Shader::~Shader() {
    this->destroyShaderModule();
}


void Shader::readFromFile(string filename) {
    ifstream shaderFile{filename, ios::ate | ios::binary};
    if (!shaderFile.is_open()) {
        cerr << "Error opening shader file" << endl;
        exit(1);
    }

    auto shaderFileSize = shaderFile.tellg();
    if (shaderFileSize == -1) {
        cerr << "Error getting shader file size" << endl;
        exit(1);
    }
    this->bytecode.resize(static_cast<size_t>(shaderFileSize));

    shaderFile.seekg(0);
    shaderFile.read(this->bytecode.data(), shaderFileSize);

    shaderFile.close();
}

void Shader::createShaderModule() {
    VkShaderModuleCreateInfo createInfo{
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = bytecode.size(),
        .pCode = reinterpret_cast<const uint32_t*>(bytecode.data()),
    };

    if (vkCreateShaderModule(this->state, &createInfo, nullptr, &this->shaderModule) != VK_SUCCESS) {
        cerr << "Cannot create shader module" << endl;
        exit(1);
    }
}

void Shader::destroyShaderModule() {
    vkDestroyShaderModule(this->state, this->shaderModule, nullptr);
}


VkPipelineShaderStageCreateInfo Shader::generatePipelineStageCreateInfo() const {
    return VkPipelineShaderStageCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = this->stage,
        .module = this->shaderModule,
        .pName = "main",
    };
}


vector<VkPipelineShaderStageCreateInfo> Shader::computePipelineShaderStageCreateInfos(std::vector<Shader> const& shaders) {
    vector<VkPipelineShaderStageCreateInfo> createInfos;

    for (auto const& shader : shaders) {
        createInfos.push_back(shader.generatePipelineStageCreateInfo());
    }

    return createInfos;
}
