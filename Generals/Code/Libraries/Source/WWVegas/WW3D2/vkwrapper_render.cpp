/*
**	Vulkan Rendering Implementation
**	BeginScene, EndScene, Draw calls, etc.
*/

#include "vkwrapper.h"
#include "wwdebug.h"
#include <algorithm>

//=============================================================================
// Command Pool and Buffers
//=============================================================================

bool VkWrapper::CreateCommandPool()
{
    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = context.graphics_family_index;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    if (vkCreateCommandPool(context.device, &poolInfo, nullptr, &context.command_pool) != VK_SUCCESS) {
        WWDEBUG_SAY(("Failed to create command pool!\n"));
        return false;
    }

    return true;
}

bool VkWrapper::CreateCommandBuffers()
{
    context.command_buffers.resize(context.swapchain_framebuffers.size());

    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = context.command_pool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t)context.command_buffers.size();

    if (vkAllocateCommandBuffers(context.device, &allocInfo, context.command_buffers.data()) != VK_SUCCESS) {
        WWDEBUG_SAY(("Failed to allocate command buffers!\n"));
        return false;
    }

    return true;
}

//=============================================================================
// Framebuffers
//=============================================================================

bool VkWrapper::CreateFramebuffers()
{
    context.swapchain_framebuffers.resize(context.swapchain_image_views.size());

    for (size_t i = 0; i < context.swapchain_image_views.size(); i++) {
        VkImageView attachments[] = {
            context.swapchain_image_views[i],
            context.depth_image_view
        };

        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = context.render_pass;
        framebufferInfo.attachmentCount = 2;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = context.swapchain_extent.width;
        framebufferInfo.height = context.swapchain_extent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(context.device, &framebufferInfo, nullptr,
                               &context.swapchain_framebuffers[i]) != VK_SUCCESS) {
            WWDEBUG_SAY(("Failed to create framebuffer!\n"));
            return false;
        }
    }

    return true;
}

//=============================================================================
// Synchronization
//=============================================================================

bool VkWrapper::CreateSyncObjects()
{
    context.image_available_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
    context.render_finished_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
    context.in_flight_fences.resize(MAX_FRAMES_IN_FLIGHT);
    context.images_in_flight.resize(context.swapchain_images.size(), VK_NULL_HANDLE);

    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (vkCreateSemaphore(context.device, &semaphoreInfo, nullptr,
                             &context.image_available_semaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(context.device, &semaphoreInfo, nullptr,
                             &context.render_finished_semaphores[i]) != VK_SUCCESS ||
            vkCreateFence(context.device, &fenceInfo, nullptr,
                         &context.in_flight_fences[i]) != VK_SUCCESS) {
            WWDEBUG_SAY(("Failed to create synchronization objects!\n"));
            return false;
        }
    }

    return true;
}

//=============================================================================
// Scene Rendering
//=============================================================================

void VkWrapper::BeginScene()
{
    vkWaitForFences(context.device, 1, &context.in_flight_fences[context.current_frame],
                   VK_TRUE, UINT64_MAX);

    VkResult result = vkAcquireNextImageKHR(context.device, context.swapchain, UINT64_MAX,
                                           context.image_available_semaphores[context.current_frame],
                                           VK_NULL_HANDLE, &context.current_image_index);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        // Swapchain is out of date, need to recreate
        ResetDevice(false);
        return;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        WWDEBUG_SAY(("Failed to acquire swap chain image!\n"));
        return;
    }

    if (context.images_in_flight[context.current_image_index] != VK_NULL_HANDLE) {
        vkWaitForFences(context.device, 1,
                       &context.images_in_flight[context.current_image_index],
                       VK_TRUE, UINT64_MAX);
    }
    context.images_in_flight[context.current_image_index] =
        context.in_flight_fences[context.current_frame];

    // Update uniform buffer
    UpdateUniformBuffer(context.current_image_index);

    // Begin command buffer recording
    VkCommandBuffer commandBuffer = context.command_buffers[context.current_image_index];

    vkResetCommandBuffer(commandBuffer, 0);

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        WWDEBUG_SAY(("Failed to begin recording command buffer!\n"));
        return;
    }

    context.in_render_pass = false;
}

void VkWrapper::EndScene(bool flip_frame)
{
    VkCommandBuffer commandBuffer = context.command_buffers[context.current_image_index];

    if (context.in_render_pass) {
        vkCmdEndRenderPass(commandBuffer);
        context.in_render_pass = false;
    }

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        WWDEBUG_SAY(("Failed to record command buffer!\n"));
        return;
    }

    if (flip_frame) {
        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = {context.image_available_semaphores[context.current_frame]};
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        VkSemaphore signalSemaphores[] = {context.render_finished_semaphores[context.current_frame]};
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        vkResetFences(context.device, 1, &context.in_flight_fences[context.current_frame]);

        if (vkQueueSubmit(context.graphics_queue, 1, &submitInfo,
                         context.in_flight_fences[context.current_frame]) != VK_SUCCESS) {
            WWDEBUG_SAY(("Failed to submit draw command buffer!\n"));
            return;
        }

        VkPresentInfoKHR presentInfo = {};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR swapChains[] = {context.swapchain};
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &context.current_image_index;

        VkResult result = vkQueuePresentKHR(context.present_queue, &presentInfo);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
            context.framebuffer_resized) {
            context.framebuffer_resized = false;
            ResetDevice(false);
        } else if (result != VK_SUCCESS) {
            WWDEBUG_SAY(("Failed to present swap chain image!\n"));
        }

        context.current_frame = (context.current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
    }

    last_frame_draw_calls = draw_calls;
    draw_calls = 0;
}

void VkWrapper::Clear(bool clear_color, bool clear_z_stencil,
                     const Vector3& color, float dest_alpha,
                     float z, unsigned stencil)
{
    if (!context.in_render_pass) {
        VkRenderPassBeginInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = context.render_pass;
        renderPassInfo.framebuffer = context.swapchain_framebuffers[context.current_image_index];
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = context.swapchain_extent;

        std::vector<VkClearValue> clearValues(2);

        if (clear_color) {
            clearValues[0].color = {{color.X, color.Y, color.Z, dest_alpha}};
        } else {
            clearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
        }

        if (clear_z_stencil) {
            clearValues[1].depthStencil = {z, stencil};
        } else {
            clearValues[1].depthStencil = {1.0f, 0};
        }

        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(context.command_buffers[context.current_image_index],
                            &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        context.in_render_pass = true;

        // Bind default pipeline
        if (current_pipeline && current_pipeline->pipeline != VK_NULL_HANDLE) {
            vkCmdBindPipeline(context.command_buffers[context.current_image_index],
                            VK_PIPELINE_BIND_POINT_GRAPHICS,
                            current_pipeline->pipeline);
        }
    }
}

//=============================================================================
// Drawing
//=============================================================================

void VkWrapper::DrawIndexedPrimitive(unsigned primitive_type,
                                     unsigned short start_index,
                                     unsigned short polygon_count,
                                     unsigned short min_vertex_index,
                                     unsigned short vertex_count)
{
    if (!context.in_render_pass) {
        WWDEBUG_SAY(("DrawIndexedPrimitive called outside render pass!\n"));
        return;
    }

    VkCommandBuffer commandBuffer = context.command_buffers[context.current_image_index];

    // Bind vertex buffer
    if (current_vertex_buffer && current_vertex_buffer->buffer != VK_NULL_HANDLE) {
        VkBuffer vertexBuffers[] = {current_vertex_buffer->buffer};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
    }

    // Bind index buffer
    if (current_index_buffer && current_index_buffer->buffer != VK_NULL_HANDLE) {
        vkCmdBindIndexBuffer(commandBuffer, current_index_buffer->buffer, 0, VK_INDEX_TYPE_UINT16);
    }

    // Bind descriptor sets
    if (descriptor_sets.size() > context.current_image_index) {
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                               current_pipeline->layout, 0, 1,
                               &descriptor_sets[context.current_image_index], 0, nullptr);
    }

    // Draw
    unsigned index_count = polygon_count * 3; // Assuming triangles
    vkCmdDrawIndexed(commandBuffer, index_count, 1, start_index, 0, 0);

    draw_calls++;
}

//=============================================================================
// State Management
//=============================================================================

void VkWrapper::SetViewport(const RenderViewport& viewport)
{
    current_viewport = viewport;

    if (context.in_render_pass) {
        VkViewport vp = {};
        vp.x = (float)viewport.x;
        vp.y = (float)viewport.y;
        vp.width = (float)viewport.width;
        vp.height = (float)viewport.height;
        vp.minDepth = viewport.minZ;
        vp.maxDepth = viewport.maxZ;

        vkCmdSetViewport(context.command_buffers[context.current_image_index], 0, 1, &vp);
    }
}

void VkWrapper::SetTransform(int transform_type, const Matrix4& m)
{
    switch (transform_type) {
        case 256: // D3DTS_WORLD
            current_world = m;
            break;
        case 2: // D3DTS_VIEW
            current_view = m;
            break;
        case 3: // D3DTS_PROJECTION
            current_projection = m;
            break;
    }
}

void VkWrapper::GetTransform(int transform_type, Matrix4& m)
{
    switch (transform_type) {
        case 256: // D3DTS_WORLD
            m = current_world;
            break;
        case 2: // D3DTS_VIEW
            m = current_view;
            break;
        case 3: // D3DTS_PROJECTION
            m = current_projection;
            break;
    }
}

void VkWrapper::SetLight(unsigned index, const RenderLight* light)
{
    if (index < MAX_VK_LIGHTS && light) {
        current_lights[index] = *light;
    }
}

void VkWrapper::EnableLight(unsigned index, bool enable)
{
    if (index < MAX_VK_LIGHTS) {
        current_light_enables[index] = enable;
    }
}

void VkWrapper::SetFog(bool enable, const Vector3& color, float start, float end)
{
    fog_enabled = enable;
    fog_color = color;
    fog_start = start;
    fog_end = end;
}

//=============================================================================
// Uniform Buffer Update
//=============================================================================

void VkWrapper::UpdateUniformBuffer(uint32_t current_image)
{
    if (current_image >= uniform_buffers.size()) return;

    VulkanUniformBuffer ubo = {};
    ubo.world = current_world;
    ubo.view = current_view;
    ubo.projection = current_projection;

    for (unsigned i = 0; i < MAX_VK_LIGHTS; i++) {
        ubo.lights[i] = current_lights[i];
        ubo.light_enables[i] = current_light_enables[i] ? 1 : 0;
    }

    ubo.fog_color = Vector4(fog_color.X, fog_color.Y, fog_color.Z, 1.0f);
    ubo.fog_start = fog_start;
    ubo.fog_end = fog_end;
    ubo.fog_enabled = fog_enabled ? 1 : 0;

    void* data;
    vkMapMemory(context.device, uniform_buffers_memory[current_image], 0, sizeof(ubo), 0, &data);
    memcpy(data, &ubo, sizeof(ubo));
    vkUnmapMemory(context.device, uniform_buffers_memory[current_image]);
}

//=============================================================================
// Statistics
//=============================================================================

unsigned VkWrapper::GetLastFrameDX8Calls()
{
    return last_frame_draw_calls;
}

void VkWrapper::ResetStatistics()
{
    draw_calls = 0;
    last_frame_draw_calls = 0;
}

//=============================================================================
// Stub implementations (to be completed)
//=============================================================================

void VkWrapper::SetLightEnvironment(LightEnvironmentClass* light_env)
{
    // TODO: Extract lights from environment and set them
}

void VkWrapper::SetMaterial(const VertexMaterialClass* material)
{
    // TODO: Update material uniforms
}

void VkWrapper::SetShader(const ShaderClass& shader)
{
    // TODO: Select appropriate pipeline based on shader
}

void VkWrapper::SetTexture(unsigned stage, TextureClass* texture)
{
    // TODO: Bind texture to descriptor set
    if (stage < MAX_VK_TEXTURE_STAGES) {
        // current_textures[stage] = texture->GetVulkanTexture();
    }
}

void VkWrapper::SetVertexBuffer(const VertexBufferClass* vb)
{
    // TODO: Get Vulkan buffer from vertex buffer class
    // current_vertex_buffer = vb->GetVulkanBuffer();
}

void VkWrapper::SetIndexBuffer(const IndexBufferClass* ib, unsigned short index_base_offset)
{
    // TODO: Get Vulkan buffer from index buffer class
    // current_index_buffer = ib->GetVulkanBuffer();
}
