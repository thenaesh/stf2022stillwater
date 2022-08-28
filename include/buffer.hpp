#ifndef _BUFFER_H
#define _BUFFER_H

#include <boilerplate.hpp>
#include <vulkanstate.hpp>


struct VertexDescriptions {
    vector<VkVertexInputBindingDescription> bindingDescriptions;
    vector<VkVertexInputAttributeDescription> attributeDescriptions;

    constexpr uint32_t numBindingDescriptions() {
        return static_cast<uint32_t>(this->bindingDescriptions.size());
    }

    constexpr uint32_t numAttributeDescriptions() {
        return static_cast<uint32_t>(this->attributeDescriptions.size());
    }

    constexpr VkVertexInputBindingDescription const* getBindingDescriptionsPtr() {
        return const_cast<VkVertexInputBindingDescription const*>(this->bindingDescriptions.data());
    }

    constexpr VkVertexInputAttributeDescription const* getAttributeDescriptionsPtr() {
        return const_cast<VkVertexInputAttributeDescription const*>(this->attributeDescriptions.data());
    }

    template <typename HeadVertexT, typename... TailVertexTs>
    static void create(
            vector<VkVertexInputBindingDescription>& bindingDescriptions,
            vector<VkVertexInputAttributeDescription>& attributeDescriptions) {

        bindingDescriptions.push_back(HeadVertexT::getBindingDescription());
        for (auto d : HeadVertexT::getAttributeDescriptions()) {
            attributeDescriptions.push_back(d);
        }

        if constexpr (sizeof... (TailVertexTs) > 0) {
            create<TailVertexTs...>(bindingDescriptions, attributeDescriptions);
        }
    }

    template <typename... VertexTs>
    static VertexDescriptions create() {
        VertexDescriptions v;
        create<VertexTs...>(v.bindingDescriptions, v.attributeDescriptions);
        return v;
    };
};


template <typename VertexT>
struct VertexBuffer {

    VulkanState& state;

    vector<VertexT> vertices;

    VkBuffer buf;
    VkDeviceMemory bufMem;


    VertexBuffer(VulkanState& state, size_t num_vertices) : state{state}, vertices{num_vertices} {
        this->initBuffer();
        this->allocateBufferMemory();
    }

    virtual ~VertexBuffer() {
        this->destroyBuffer();  // TODO: queue this after swapchain destruction
        this->freeBufferMemory();
    }


    operator VkBuffer () { return this->buf; }

    constexpr size_t numVertices() {
        return this->vertices.size();
    }

    constexpr VkDeviceSize size() {
        return sizeof (VertexT) * this->numVertices();
    };


    void setVertices(initializer_list<VertexT> vs) {
        this->vertices.clear();
        this->vertices.insert(this->vertices.end(), vs.begin(), vs.end());
    }

    void setVertices(vector<VertexT> vs) {
        this->vertices.clear();
        this->vertices.insert(this->vertices.end(), vs.begin(), vs.end());
    }

    void syncWithGpuMemory() {
        void* gpuMem;

        vkMapMemory(
            this->state,
            this->bufMem,
            0,
            this->size(),
            0,
            &gpuMem);

        memcpy(gpuMem, this->vertices.data(), static_cast<size_t>(this->size()));

        vkUnmapMemory(this->state, this->bufMem);
    }


    void initBuffer() {
        VkBufferCreateInfo bufferInfo{
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = this->size(),
            .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        };

        if (vkCreateBuffer(
            this->state,
            &bufferInfo,
            nullptr,
            &this->buf) != VK_SUCCESS) {
                cerr << "Failed to create vertex buffer" << endl;
                exit(1);
        }
    }

    void destroyBuffer() {
        vkDestroyBuffer(state, this->buf, nullptr);

    }

    void allocateBufferMemory() {
        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(
            this->state,
            this->buf,
            &memRequirements);

        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(
            this->state,
            &memProperties);

        VkMemoryAllocateInfo allocInfo{
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .allocationSize = memRequirements.size,
            .memoryTypeIndex = this->state.findMemoryType(
                memRequirements.memoryTypeBits,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT),
        };

        if (vkAllocateMemory(
                    this->state,
                    &allocInfo,
                    nullptr,
                    &this->bufMem) != VK_SUCCESS) {
            cerr << "Failed to allocate vertex buffer memory" << endl;
            exit(1);
        }

        vkBindBufferMemory(
            this->state,
            this->buf,
            this->bufMem,
            0);
        }

    void freeBufferMemory() {
        vkFreeMemory(this->state, this->bufMem, nullptr);
    }
};


#endif
