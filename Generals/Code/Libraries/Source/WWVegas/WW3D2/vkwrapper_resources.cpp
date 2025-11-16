/*
**	Vulkan Resource Management
**	Buffers, textures, depth resources, uniform buffers
*/

#include "vkwrapper.h"
#include "wwdebug.h"
#include <cstring>

//=============================================================================
// Memory Utilities
//=============================================================================

uint32_t VkWrapper::FindMemoryType(uint32_t type_filter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(context.physical_device, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((type_filter & (1 << i)) &&
            (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    WWDEBUG_SAY(("Failed to find suitable memory type!\n"));
    return 0;
}

//=============================================================================
// Buffer Creation
//=============================================================================

bool VkWrapper::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
                             VkMemoryPropertyFlags properties,
                             VkBuffer& buffer, VkDeviceMemory& buffer_memory)
{
    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(context.device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
        WWDEBUG_SAY(("Failed to create buffer!\n"));
        return false;
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(context.device, buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(context.device, &allocInfo, nullptr, &buffer_memory) != VK_SUCCESS) {
        WWDEBUG_SAY(("Failed to allocate buffer memory!\n"));
        return false;
    }

    vkBindBufferMemory(context.device, buffer, buffer_memory, 0);

    return true;
}

void VkWrapper::CopyBuffer(VkBuffer src_buffer, VkBuffer dst_buffer, VkDeviceSize size)
{
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = context.command_pool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(context.device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    VkBufferCopy copyRegion = {};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, src_buffer, dst_buffer, 1, &copyRegion);

    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(context.graphics_queue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(context.graphics_queue);

    vkFreeCommandBuffers(context.device, context.command_pool, 1, &commandBuffer);
}

//=============================================================================
// Depth Resources
//=============================================================================

bool VkWrapper::CreateDepthResources()
{
    VkFormat depthFormat = FindDepthFormat();
    context.depth_format = depthFormat;

    if (!CreateImage(context.swapchain_extent.width,
                    context.swapchain_extent.height,
                    1,
                    depthFormat,
                    VK_IMAGE_TILING_OPTIMAL,
                    VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                    context.depth_image,
                    context.depth_image_memory)) {
        return false;
    }

    context.depth_image_view = CreateImageView(context.depth_image, depthFormat,
                                               VK_IMAGE_ASPECT_DEPTH_BIT, 1);

    if (context.depth_image_view == VK_NULL_HANDLE) {
        return false;
    }

    TransitionImageLayout(context.depth_image, depthFormat,
                         VK_IMAGE_LAYOUT_UNDEFINED,
                         VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                         1);

    return true;
}

//=============================================================================
// Image Creation
//=============================================================================

bool VkWrapper::CreateImage(uint32_t width, uint32_t height, uint32_t mip_levels,
                            VkFormat format, VkImageTiling tiling,
                            VkImageUsageFlags usage, VkMemoryPropertyFlags properties,
                            VkImage& image, VkDeviceMemory& image_memory)
{
    VkImageCreateInfo imageInfo = {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = mip_levels;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateImage(context.device, &imageInfo, nullptr, &image) != VK_SUCCESS) {
        WWDEBUG_SAY(("Failed to create image!\n"));
        return false;
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(context.device, image, &memRequirements);

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(context.device, &allocInfo, nullptr, &image_memory) != VK_SUCCESS) {
        WWDEBUG_SAY(("Failed to allocate image memory!\n"));
        return false;
    }

    vkBindImageMemory(context.device, image, image_memory, 0);

    return true;
}

VkImageView VkWrapper::CreateImageView(VkImage image, VkFormat format,
                                      VkImageAspectFlags aspect_flags,
                                      uint32_t mip_levels)
{
    VkImageViewCreateInfo viewInfo = {};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = aspect_flags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = mip_levels;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    VkImageView imageView;
    if (vkCreateImageView(context.device, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
        WWDEBUG_SAY(("Failed to create texture image view!\n"));
        return VK_NULL_HANDLE;
    }

    return imageView;
}

void VkWrapper::TransitionImageLayout(VkImage image, VkFormat format,
                                     VkImageLayout old_layout, VkImageLayout new_layout,
                                     uint32_t mip_levels)
{
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = context.command_pool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(context.device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    VkImageMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = old_layout;
    barrier.newLayout = new_layout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = mip_levels;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    if (new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

        if (format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT) {
            barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }
    }

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED &&
        new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
               new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED &&
               new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                               VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    } else {
        sourceStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
        destinationStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    }

    vkCmdPipelineBarrier(commandBuffer,
                        sourceStage, destinationStage,
                        0,
                        0, nullptr,
                        0, nullptr,
                        1, &barrier);

    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(context.graphics_queue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(context.graphics_queue);

    vkFreeCommandBuffers(context.device, context.command_pool, 1, &commandBuffer);
}

//=============================================================================
// Uniform Buffers
//=============================================================================

bool VkWrapper::CreateUniformBuffers()
{
    VkDeviceSize bufferSize = sizeof(VulkanUniformBuffer);

    uniform_buffers.resize(context.swapchain_images.size());
    uniform_buffers_memory.resize(context.swapchain_images.size());

    for (size_t i = 0; i < context.swapchain_images.size(); i++) {
        if (!CreateBuffer(bufferSize,
                         VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                         uniform_buffers[i],
                         uniform_buffers_memory[i])) {
            return false;
        }
    }

    return true;
}

//=============================================================================
// Descriptor Pool and Sets
//=============================================================================

bool VkWrapper::CreateDescriptorPool()
{
    std::vector<VkDescriptorPoolSize> poolSizes(2);
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = static_cast<uint32_t>(context.swapchain_images.size());
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = static_cast<uint32_t>(context.swapchain_images.size());

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = static_cast<uint32_t>(context.swapchain_images.size());

    if (vkCreateDescriptorPool(context.device, &poolInfo, nullptr, &context.descriptor_pool) != VK_SUCCESS) {
        WWDEBUG_SAY(("Failed to create descriptor pool!\n"));
        return false;
    }

    return true;
}

bool VkWrapper::CreateDescriptorSets()
{
    std::vector<VkDescriptorSetLayout> layouts(context.swapchain_images.size(),
                                               context.descriptor_set_layout);

    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = context.descriptor_pool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(context.swapchain_images.size());
    allocInfo.pSetLayouts = layouts.data();

    descriptor_sets.resize(context.swapchain_images.size());
    if (vkAllocateDescriptorSets(context.device, &allocInfo, descriptor_sets.data()) != VK_SUCCESS) {
        WWDEBUG_SAY(("Failed to allocate descriptor sets!\n"));
        return false;
    }

    for (size_t i = 0; i < context.swapchain_images.size(); i++) {
        VkDescriptorBufferInfo bufferInfo = {};
        bufferInfo.buffer = uniform_buffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(VulkanUniformBuffer);

        VkWriteDescriptorSet descriptorWrite = {};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = descriptor_sets[i];
        descriptorWrite.dstBinding = 0;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = &bufferInfo;

        vkUpdateDescriptorSets(context.device, 1, &descriptorWrite, 0, nullptr);
    }

    return true;
}

//=============================================================================
// Swapchain Cleanup
//=============================================================================

void VkWrapper::CleanupSwapchain()
{
    if (context.depth_image_view != VK_NULL_HANDLE) {
        vkDestroyImageView(context.device, context.depth_image_view, nullptr);
        context.depth_image_view = VK_NULL_HANDLE;
    }

    if (context.depth_image != VK_NULL_HANDLE) {
        vkDestroyImage(context.device, context.depth_image, nullptr);
        context.depth_image = VK_NULL_HANDLE;
    }

    if (context.depth_image_memory != VK_NULL_HANDLE) {
        vkFreeMemory(context.device, context.depth_image_memory, nullptr);
        context.depth_image_memory = VK_NULL_HANDLE;
    }

    for (auto framebuffer : context.swapchain_framebuffers) {
        vkDestroyFramebuffer(context.device, framebuffer, nullptr);
    }
    context.swapchain_framebuffers.clear();

    if (context.command_pool != VK_NULL_HANDLE && !context.command_buffers.empty()) {
        vkFreeCommandBuffers(context.device, context.command_pool,
                            static_cast<uint32_t>(context.command_buffers.size()),
                            context.command_buffers.data());
        context.command_buffers.clear();
    }

    for (auto imageView : context.swapchain_image_views) {
        vkDestroyImageView(context.device, imageView, nullptr);
    }
    context.swapchain_image_views.clear();

    if (context.swapchain != VK_NULL_HANDLE) {
        vkDestroySwapchainKHR(context.device, context.swapchain, nullptr);
        context.swapchain = VK_NULL_HANDLE;
    }

    for (size_t i = 0; i < uniform_buffers.size(); i++) {
        vkDestroyBuffer(context.device, uniform_buffers[i], nullptr);
        vkFreeMemory(context.device, uniform_buffers_memory[i], nullptr);
    }
    uniform_buffers.clear();
    uniform_buffers_memory.clear();

    if (context.descriptor_pool != VK_NULL_HANDLE) {
        vkDestroyDescriptorPool(context.device, context.descriptor_pool, nullptr);
        context.descriptor_pool = VK_NULL_HANDLE;
    }
    descriptor_sets.clear();
}

//=============================================================================
// Public Resource Creation (RenderBackend interface)
//=============================================================================

void* VkWrapper::CreateVertexBuffer(unsigned size, unsigned format)
{
    VkBufferWrapper* wrapper = new VkBufferWrapper();
    wrapper->size = size;

    if (!CreateBuffer(size,
                     VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                     VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                     wrapper->buffer,
                     wrapper->memory)) {
        delete wrapper;
        return nullptr;
    }

    return wrapper;
}

void* VkWrapper::CreateIndexBuffer(unsigned size)
{
    VkBufferWrapper* wrapper = new VkBufferWrapper();
    wrapper->size = size;

    if (!CreateBuffer(size,
                     VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                     VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                     wrapper->buffer,
                     wrapper->memory)) {
        delete wrapper;
        return nullptr;
    }

    return wrapper;
}

void* VkWrapper::CreateTexture(unsigned width, unsigned height,
                               unsigned format, unsigned mip_levels)
{
    VkTextureWrapper* wrapper = new VkTextureWrapper();
    wrapper->width = width;
    wrapper->height = height;
    wrapper->mip_levels = mip_levels;
    wrapper->format = VK_FORMAT_R8G8B8A8_UNORM; // Default format

    if (!CreateImage(width, height, mip_levels,
                    wrapper->format,
                    VK_IMAGE_TILING_OPTIMAL,
                    VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                    wrapper->image,
                    wrapper->memory)) {
        delete wrapper;
        return nullptr;
    }

    wrapper->image_view = CreateImageView(wrapper->image, wrapper->format,
                                          VK_IMAGE_ASPECT_COLOR_BIT, mip_levels);

    if (wrapper->image_view == VK_NULL_HANDLE) {
        vkDestroyImage(context.device, wrapper->image, nullptr);
        vkFreeMemory(context.device, wrapper->memory, nullptr);
        delete wrapper;
        return nullptr;
    }

    return wrapper;
}

void VkWrapper::DestroyVertexBuffer(void* buffer)
{
    VkBufferWrapper* wrapper = static_cast<VkBufferWrapper*>(buffer);
    if (wrapper) {
        if (wrapper->buffer != VK_NULL_HANDLE) {
            vkDestroyBuffer(context.device, wrapper->buffer, nullptr);
        }
        if (wrapper->memory != VK_NULL_HANDLE) {
            vkFreeMemory(context.device, wrapper->memory, nullptr);
        }
        delete wrapper;
    }
}

void VkWrapper::DestroyIndexBuffer(void* buffer)
{
    DestroyVertexBuffer(buffer); // Same implementation
}

void VkWrapper::DestroyTexture(void* texture)
{
    VkTextureWrapper* wrapper = static_cast<VkTextureWrapper*>(texture);
    if (wrapper) {
        if (wrapper->sampler != VK_NULL_HANDLE) {
            vkDestroySampler(context.device, wrapper->sampler, nullptr);
        }
        if (wrapper->image_view != VK_NULL_HANDLE) {
            vkDestroyImageView(context.device, wrapper->image_view, nullptr);
        }
        if (wrapper->image != VK_NULL_HANDLE) {
            vkDestroyImage(context.device, wrapper->image, nullptr);
        }
        if (wrapper->memory != VK_NULL_HANDLE) {
            vkFreeMemory(context.device, wrapper->memory, nullptr);
        }
        delete wrapper;
    }
}
