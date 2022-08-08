#include <pipeline.hpp>

using namespace std;


Pipeline::Pipeline(VulkanState const& state, vector<Shader> shaders) : state{state}, shaders{move(shaders)} {
    this->createRenderPass();
    this->createPipelineLayout();
    this->createPipeline();
    this->createFramebuffers();
    this->createSyncPrimitives();

    this->allocateCommandBuffer();
}

Pipeline::~Pipeline() {
    this->destroySyncPrimitives();
    this->destroyFramebuffers();
    this->destroyPipeline();
    this->destroyPipelineLayout();
    this->destroyRenderPass();
}


void Pipeline::createRenderPass() {
    VkAttachmentDescription colorAttachment{
        .format = this->state.swapChainImageFormat,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
    };

    VkAttachmentReference colorAttachmentRef{
        .attachment = 0,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    };

    VkSubpassDescription subpass{
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .colorAttachmentCount = 1,
        .pColorAttachments = &colorAttachmentRef,
    };

    VkSubpassDependency dependency{
      .srcSubpass = VK_SUBPASS_EXTERNAL,
      .dstSubpass = 0,
      .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
      .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
      .srcAccessMask = 0,
      .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
    };

    VkRenderPassCreateInfo renderPassInfo{
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = 1,
        .pAttachments = &colorAttachment,
        .subpassCount = 1,
        .pSubpasses = &subpass,
        .dependencyCount = 1,
        .pDependencies = &dependency,
    };

    if (vkCreateRenderPass(this->state, &renderPassInfo, nullptr, &this->renderPass) !=
        VK_SUCCESS) {
            cerr << "Failed to create render pass" << endl;
            exit(1);
    }
}

void Pipeline::destroyRenderPass() {
    vkDestroyRenderPass(this->state, this->renderPass, nullptr);
}


void Pipeline::createPipelineLayout() {

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = 0, // Optional
        .pSetLayouts = nullptr, // Optional
        .pushConstantRangeCount = 0, // Optional
        .pPushConstantRanges = nullptr, // Optiona
    };
    if (vkCreatePipelineLayout(this->state, &pipelineLayoutInfo, nullptr, &this->pipelineLayout) != VK_SUCCESS) {
        cerr << "Unable to create pipeline layout" << endl;
        exit(1);
    }
}

void Pipeline::destroyPipelineLayout() {
    vkDestroyPipelineLayout(this->state, this->pipelineLayout, nullptr);
}


void Pipeline::createPipeline() {
    VkPipelineVertexInputStateCreateInfo vertexInputInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = 0,
        .pVertexBindingDescriptions = nullptr, // Optional
        .vertexAttributeDescriptionCount = 0,
        .pVertexAttributeDescriptions = nullptr, // Optional
    };

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE,
    };

    VkViewport viewport{
        .x = 0.0f,
        .y = 0.0f,
        .width = (float) this->state.swapChainExtent.width,
        .height = (float) this->state.swapChainExtent.height,
        .minDepth = 0.0f,
        .maxDepth = 1.0f,
    };

    VkRect2D scissor{
        .offset = {0, 0},
        .extent = this->state.swapChainExtent,
    };

    std::vector<VkDynamicState> dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR,
    };

    VkPipelineDynamicStateCreateInfo dynamicState{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
        .pDynamicStates = dynamicStates.data(),
    };

    VkPipelineViewportStateCreateInfo viewportState{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .scissorCount = 1,
    };

    VkPipelineRasterizationStateCreateInfo rasterizer{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .depthClampEnable = VK_FALSE,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .cullMode = VK_CULL_MODE_BACK_BIT,
        .frontFace = VK_FRONT_FACE_CLOCKWISE,
        .depthBiasEnable = VK_FALSE,
        .depthBiasConstantFactor = 0.0f, // Optional
        .depthBiasClamp = 0.0f, // Optional
        .depthBiasSlopeFactor = 0.0f, // Optional
        .lineWidth = 1.0f,
    };

    VkPipelineMultisampleStateCreateInfo multisampling{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
        .sampleShadingEnable = VK_FALSE,
        .minSampleShading = 1.0f, // Optional
        .pSampleMask = nullptr, // Optional
        .alphaToCoverageEnable = VK_FALSE, // Optional
        .alphaToOneEnable = VK_FALSE, // Optional
    };

    VkPipelineColorBlendAttachmentState colorBlendAttachment{
        .blendEnable = VK_FALSE,
        .srcColorBlendFactor = VK_BLEND_FACTOR_ONE, // Optional
        .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO, // Optional
        .colorBlendOp = VK_BLEND_OP_ADD, // Optional
        .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE, // Optional
        .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO, // Optional
        .alphaBlendOp = VK_BLEND_OP_ADD, // Optional
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
    };

    VkPipelineColorBlendStateCreateInfo colorBlending{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .logicOpEnable = VK_FALSE,
        .logicOp = VK_LOGIC_OP_COPY, // Optional
        .attachmentCount = 1,
        .pAttachments = &colorBlendAttachment,
        .blendConstants = {
            0.0f,
            0.0f,
            0.0f,
            0.0f,
        },
    };

    auto shaderStages = Shader::computePipelineShaderStageCreateInfos(this->shaders);

    VkGraphicsPipelineCreateInfo pipelineInfo{
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .stageCount = static_cast<uint32_t>(shaderStages.size()),
        .pStages = shaderStages.data(),
        .pVertexInputState = &vertexInputInfo,
        .pInputAssemblyState = &inputAssembly,
        .pViewportState = &viewportState,
        .pRasterizationState = &rasterizer,
        .pMultisampleState = &multisampling,
        .pDepthStencilState = nullptr, // Optional
        .pColorBlendState = &colorBlending,
        .pDynamicState = &dynamicState,
        .layout = this->pipelineLayout,
        .renderPass = this->renderPass,
        .subpass = 0,
        .basePipelineHandle = VK_NULL_HANDLE, // Optional
        .basePipelineIndex = -1,              // Optional
    };

    if (vkCreateGraphicsPipelines(
        this->state,
        VK_NULL_HANDLE,
        1,
        &pipelineInfo,
        nullptr,
        &this->pipeline
    ) != VK_SUCCESS) {
        cerr << "Unable to create graphics pipeline" << endl;
        exit(1);
    }
}

void Pipeline::destroyPipeline() {
    vkDestroyPipeline(state, this->pipeline, nullptr);
}


void Pipeline::createFramebuffers() {
    this->framebuffers.resize(this->state.imageViews.size());

    for (size_t i = 0; i < this->state.imageViews.size(); i++) {
        VkImageView attachments[] = {this->state.imageViews[i]};

        VkFramebufferCreateInfo framebufferInfo{
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass = this->renderPass,
            .attachmentCount = 1,
            .pAttachments = attachments,
            .width = this->state.swapChainExtent.width,
            .height = this->state.swapChainExtent.height,
            .layers = 1,
        };

        if (vkCreateFramebuffer(
            this->state,
            &framebufferInfo,
            nullptr,
            &this->framebuffers[i]) != VK_SUCCESS) {
                cerr << "Unable to create framebuffers" << endl;
                exit(1);
        }
    }
}

void Pipeline::destroyFramebuffers() {
    for (auto fb : this->framebuffers) {
        vkDestroyFramebuffer(this->state, fb, nullptr);
    }
}


void Pipeline::createSyncPrimitives() {
    VkSemaphoreCreateInfo semaphoreInfo{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
    };

    VkFenceCreateInfo fenceInfo{
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT,
    };

    auto createSemaphore = [this, semaphoreInfo](VkSemaphore* pSemaphore){
        return vkCreateSemaphore(
            this->state,
            &semaphoreInfo,
            nullptr,
            pSemaphore) == VK_SUCCESS;
    };

    auto createFence = [this, fenceInfo](VkFence* pFence){
        return vkCreateFence(
            this->state,
            &fenceInfo,
            nullptr,
            pFence) == VK_SUCCESS;
    };

    if (!(createSemaphore(&this->imageAvailableSemaphore) && createSemaphore(&this->renderFinishedSemaphore) && createFence(&this->inFlightFence))) {
        cerr << "Failed to create synchronization primitives for rendering" << endl;
        exit(1);
    }
}

void Pipeline::destroySyncPrimitives() {
    vkDestroyFence(this->state, this->inFlightFence, nullptr);
    vkDestroySemaphore(this->state, this->renderFinishedSemaphore, nullptr);
    vkDestroySemaphore(this->state, this->imageAvailableSemaphore, nullptr);
}


void Pipeline::allocateCommandBuffer() {
    VkCommandBufferAllocateInfo allocInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = this->state,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };

    if (vkAllocateCommandBuffers(
        this->state,
        &allocInfo,
        &this->commandBuffer) != VK_SUCCESS) {
            cerr << "Unable to allocate command buffer" << endl;
            exit(1);
        }
}


void Pipeline::recordRenderPass(CommandBufferRecorder f, uint32_t imageIndex) {
    VkCommandBufferBeginInfo beginInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = 0,
        .pInheritanceInfo = nullptr,
    };

    if (vkBeginCommandBuffer(this->commandBuffer, &beginInfo) != VK_SUCCESS) {
        cerr << "Failed to begin recording render pass commands" << endl;
        exit(1);
    }

    VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};

    VkRenderPassBeginInfo renderPassInfo{
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = this->renderPass,
        .framebuffer = this->framebuffers[imageIndex],
        .renderArea = VkRect2D{
            .offset = VkOffset2D{0, 0},
            .extent = this->state.swapChainExtent,
        },
        .clearValueCount = 1,
        .pClearValues = &clearColor,
    };

    vkCmdBeginRenderPass(
        commandBuffer,
        &renderPassInfo,
        VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(
        commandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        this->pipeline);

    f(this->commandBuffer, this->state);

    vkCmdEndRenderPass(this->commandBuffer);

    if (vkEndCommandBuffer(this->commandBuffer) != VK_SUCCESS) {
        cerr << "Failed to record render pass commands" << endl;
        exit(1);
    }
}

void Pipeline::render(CommandBufferRecorder f) {
    // wait for previous frame to finish rendering
    vkWaitForFences(this->state, 1, &this->inFlightFence, VK_TRUE, UINT64_MAX);
    // acquire fence by clearing signal from it
    vkResetFences(this->state, 1, &this->inFlightFence);

    uint32_t imageIndex;
    vkAcquireNextImageKHR(
        this->state,
        this->state,
        UINT64_MAX,
        this->imageAvailableSemaphore,
        VK_NULL_HANDLE,
        &imageIndex);

    vkResetCommandBuffer(this->commandBuffer, 0);
    this->recordRenderPass(f, imageIndex);

    VkSemaphore signalSemaphores[] = {this->renderFinishedSemaphore};
    VkSemaphore waitSemaphores[] = {this->imageAvailableSemaphore};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

    VkSubmitInfo submitInfo{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = waitSemaphores,
        .pWaitDstStageMask = waitStages,
        .commandBufferCount = 1,
        .pCommandBuffers = &this->commandBuffer,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = signalSemaphores,
    };

    if (vkQueueSubmit(this->state.queue, 1, &submitInfo, this->inFlightFence)) {
        cerr << "Failed to submit render pass buffer" << endl;
        exit(1);
    }

    VkSwapchainKHR swapChains[] = {this->state};

    VkPresentInfoKHR presentInfo{
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = signalSemaphores,
        .swapchainCount = 1,
        .pSwapchains = swapChains,
        .pImageIndices = &imageIndex,
        .pResults = nullptr,
    };

    vkQueuePresentKHR(this->state.presentQueue, &presentInfo);
}

void Pipeline::waitIdle() {
    vkDeviceWaitIdle(this->state);
}
