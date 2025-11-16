/*
**	Command & Conquer Generals(tm)
**	Copyright 2025 Electronic Arts Inc.
**
**	Vulkan Rendering Backend Implementation
*/

#include "vkwrapper.h"
#include "wwdebug.h"
#include "vertmaterial.h"
#include "shader.h"
#include "texture.h"

#include <cstring>
#include <set>
#include <algorithm>

// Validation layers for debug builds
#ifdef WWDEBUG
const bool enableValidationLayers = true;
#else
const bool enableValidationLayers = false;
#endif

const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

// Vulkan function pointers (loaded dynamically)
#define VK_FUNC(name) PFN_##name name = nullptr

// Instance functions
VK_FUNC(vkCreateInstance);
VK_FUNC(vkDestroyInstance);
VK_FUNC(vkEnumeratePhysicalDevices);
VK_FUNC(vkGetPhysicalDeviceProperties);
VK_FUNC(vkGetPhysicalDeviceFeatures);
VK_FUNC(vkGetPhysicalDeviceQueueFamilyProperties);
VK_FUNC(vkCreateDevice);
VK_FUNC(vkDestroyDevice);
VK_FUNC(vkGetDeviceQueue);
VK_FUNC(vkCreateSwapchainKHR);
VK_FUNC(vkDestroySwapchainKHR);
VK_FUNC(vkGetSwapchainImagesKHR);
VK_FUNC(vkCreateImageView);
VK_FUNC(vkDestroyImageView);
VK_FUNC(vkCreateRenderPass);
VK_FUNC(vkDestroyRenderPass);
VK_FUNC(vkCreateFramebuffer);
VK_FUNC(vkDestroyFramebuffer);
VK_FUNC(vkCreateCommandPool);
VK_FUNC(vkDestroyCommandPool);
VK_FUNC(vkAllocateCommandBuffers);
VK_FUNC(vkFreeCommandBuffers);
VK_FUNC(vkBeginCommandBuffer);
VK_FUNC(vkEndCommandBuffer);
VK_FUNC(vkCmdBeginRenderPass);
VK_FUNC(vkCmdEndRenderPass);
VK_FUNC(vkCmdBindPipeline);
VK_FUNC(vkCmdDraw);
VK_FUNC(vkCmdDrawIndexed);
VK_FUNC(vkCreateSemaphore);
VK_FUNC(vkDestroySemaphore);
VK_FUNC(vkCreateFence);
VK_FUNC(vkDestroyFence);
VK_FUNC(vkWaitForFences);
VK_FUNC(vkResetFences);
VK_FUNC(vkAcquireNextImageKHR);
VK_FUNC(vkQueueSubmit);
VK_FUNC(vkQueuePresentKHR);
VK_FUNC(vkQueueWaitIdle);
VK_FUNC(vkDeviceWaitIdle);

#undef VK_FUNC

//=============================================================================
// VulkanContext
//=============================================================================

VulkanContext::VulkanContext()
    : instance(VK_NULL_HANDLE)
    , physical_device(VK_NULL_HANDLE)
    , device(VK_NULL_HANDLE)
    , graphics_queue(VK_NULL_HANDLE)
    , present_queue(VK_NULL_HANDLE)
    , graphics_family_index(0)
    , present_family_index(0)
    , swapchain(VK_NULL_HANDLE)
    , swapchain_format(VK_FORMAT_UNDEFINED)
    , render_pass(VK_NULL_HANDLE)
    , command_pool(VK_NULL_HANDLE)
    , current_frame(0)
    , depth_image(VK_NULL_HANDLE)
    , depth_image_memory(VK_NULL_HANDLE)
    , depth_image_view(VK_NULL_HANDLE)
    , depth_format(VK_FORMAT_UNDEFINED)
    , descriptor_pool(VK_NULL_HANDLE)
    , descriptor_set_layout(VK_NULL_HANDLE)
    , debug_messenger(VK_NULL_HANDLE)
    , in_render_pass(false)
    , framebuffer_resized(false)
    , current_image_index(0)
{
    swapchain_extent.width = 0;
    swapchain_extent.height = 0;
}

VulkanContext::~VulkanContext()
{
}

//=============================================================================
// VkBufferWrapper
//=============================================================================

VkBufferWrapper::VkBufferWrapper()
    : buffer(VK_NULL_HANDLE)
    , memory(VK_NULL_HANDLE)
    , size(0)
    , mapped_data(nullptr)
{
}

VkBufferWrapper::~VkBufferWrapper()
{
}

//=============================================================================
// VkTextureWrapper
//=============================================================================

VkTextureWrapper::VkTextureWrapper()
    : image(VK_NULL_HANDLE)
    , memory(VK_NULL_HANDLE)
    , image_view(VK_NULL_HANDLE)
    , sampler(VK_NULL_HANDLE)
    , width(0)
    , height(0)
    , mip_levels(1)
    , format(VK_FORMAT_UNDEFINED)
{
}

VkTextureWrapper::~VkTextureWrapper()
{
}

//=============================================================================
// VulkanPipeline
//=============================================================================

VulkanPipeline::VulkanPipeline()
    : pipeline(VK_NULL_HANDLE)
    , layout(VK_NULL_HANDLE)
    , state_hash(0)
{
}

VulkanPipeline::~VulkanPipeline()
{
}

//=============================================================================
// VkWrapper
//=============================================================================

VkWrapper::VkWrapper()
    : window_handle(nullptr)
    , current_vertex_buffer(nullptr)
    , current_index_buffer(nullptr)
    , current_pipeline(nullptr)
    , draw_calls(0)
    , last_frame_draw_calls(0)
    , surface(VK_NULL_HANDLE)
    , resolution_width(800)
    , resolution_height(600)
    , fog_enabled(false)
    , fog_start(0.0f)
    , fog_end(1000.0f)
{
    // Initialize textures
    for (unsigned i = 0; i < MAX_VK_TEXTURE_STAGES; ++i) {
        current_textures[i] = nullptr;
    }

    // Initialize lights
    for (unsigned i = 0; i < MAX_VK_LIGHTS; ++i) {
        current_light_enables[i] = false;
        memset(&current_lights[i], 0, sizeof(RenderLight));
    }

    // Initialize matrices to identity
    current_world.Make_Identity();
    current_view.Make_Identity();
    current_projection.Make_Identity();

    fog_color = Vector3(0.5f, 0.5f, 0.5f);
}

VkWrapper::~VkWrapper()
{
    Shutdown();
}

//=============================================================================
// Initialization
//=============================================================================

bool VkWrapper::Init(void* hwnd)
{
    WWDEBUG_SAY(("VkWrapper::Init - Initializing Vulkan backend\n"));

    window_handle = hwnd;

    if (!CreateInstance()) {
        WWDEBUG_SAY(("Failed to create Vulkan instance\n"));
        return false;
    }

    if (enableValidationLayers && !SetupDebugMessenger()) {
        WWDEBUG_SAY(("Failed to setup debug messenger\n"));
    }

    // Create window surface (platform-specific)
#ifdef _WIN32
    VkWin32SurfaceCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    createInfo.hwnd = (HWND)hwnd;
    createInfo.hinstance = GetModuleHandle(nullptr);

    if (vkCreateWin32SurfaceKHR(context.instance, &createInfo, nullptr, &surface) != VK_SUCCESS) {
        WWDEBUG_SAY(("Failed to create window surface\n"));
        return false;
    }
#else
    // Linux/X11 surface creation would go here
    WWDEBUG_SAY(("Non-Windows surface creation not yet implemented\n"));
    return false;
#endif

    if (!PickPhysicalDevice()) {
        WWDEBUG_SAY(("Failed to pick physical device\n"));
        return false;
    }

    if (!CreateLogicalDevice()) {
        WWDEBUG_SAY(("Failed to create logical device\n"));
        return false;
    }

    if (!CreateSwapchain()) {
        WWDEBUG_SAY(("Failed to create swapchain\n"));
        return false;
    }

    if (!CreateImageViews()) {
        WWDEBUG_SAY(("Failed to create image views\n"));
        return false;
    }

    if (!CreateRenderPass()) {
        WWDEBUG_SAY(("Failed to create render pass\n"));
        return false;
    }

    if (!CreateDescriptorSetLayout()) {
        WWDEBUG_SAY(("Failed to create descriptor set layout\n"));
        return false;
    }

    if (!CreateGraphicsPipeline()) {
        WWDEBUG_SAY(("Failed to create graphics pipeline\n"));
        return false;
    }

    if (!CreateCommandPool()) {
        WWDEBUG_SAY(("Failed to create command pool\n"));
        return false;
    }

    if (!CreateDepthResources()) {
        WWDEBUG_SAY(("Failed to create depth resources\n"));
        return false;
    }

    if (!CreateFramebuffers()) {
        WWDEBUG_SAY(("Failed to create framebuffers\n"));
        return false;
    }

    if (!CreateUniformBuffers()) {
        WWDEBUG_SAY(("Failed to create uniform buffers\n"));
        return false;
    }

    if (!CreateDescriptorPool()) {
        WWDEBUG_SAY(("Failed to create descriptor pool\n"));
        return false;
    }

    if (!CreateDescriptorSets()) {
        WWDEBUG_SAY(("Failed to create descriptor sets\n"));
        return false;
    }

    if (!CreateCommandBuffers()) {
        WWDEBUG_SAY(("Failed to create command buffers\n"));
        return false;
    }

    if (!CreateSyncObjects()) {
        WWDEBUG_SAY(("Failed to create sync objects\n"));
        return false;
    }

    WWDEBUG_SAY(("VkWrapper::Init - Vulkan backend initialized successfully\n"));
    return true;
}

void VkWrapper::Shutdown()
{
    if (context.device != VK_NULL_HANDLE) {
        vkDeviceWaitIdle(context.device);
    }

    // Cleanup pipelines
    for (auto& pair : pipeline_cache) {
        if (pair.second) {
            if (pair.second->pipeline != VK_NULL_HANDLE) {
                vkDestroyPipeline(context.device, pair.second->pipeline, nullptr);
            }
            if (pair.second->layout != VK_NULL_HANDLE) {
                vkDestroyPipelineLayout(context.device, pair.second->layout, nullptr);
            }
            delete pair.second;
        }
    }
    pipeline_cache.clear();

    CleanupSwapchain();

    // Cleanup descriptor set layout
    if (context.descriptor_set_layout != VK_NULL_HANDLE) {
        vkDestroyDescriptorSetLayout(context.device, context.descriptor_set_layout, nullptr);
    }

    // Cleanup uniform buffers
    for (size_t i = 0; i < uniform_buffers.size(); i++) {
        vkDestroyBuffer(context.device, uniform_buffers[i], nullptr);
        vkFreeMemory(context.device, uniform_buffers_memory[i], nullptr);
    }

    // Cleanup sync objects
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (context.render_finished_semaphores.size() > i) {
            vkDestroySemaphore(context.device, context.render_finished_semaphores[i], nullptr);
        }
        if (context.image_available_semaphores.size() > i) {
            vkDestroySemaphore(context.device, context.image_available_semaphores[i], nullptr);
        }
        if (context.in_flight_fences.size() > i) {
            vkDestroyFence(context.device, context.in_flight_fences[i], nullptr);
        }
    }

    if (context.command_pool != VK_NULL_HANDLE) {
        vkDestroyCommandPool(context.device, context.command_pool, nullptr);
    }

    if (context.device != VK_NULL_HANDLE) {
        vkDestroyDevice(context.device, nullptr);
    }

    if (surface != VK_NULL_HANDLE) {
        vkDestroySurfaceKHR(context.instance, surface, nullptr);
    }

    if (enableValidationLayers && context.debug_messenger != VK_NULL_HANDLE) {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
            context.instance, "vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr) {
            func(context.instance, context.debug_messenger, nullptr);
        }
    }

    if (context.instance != VK_NULL_HANDLE) {
        vkDestroyInstance(context.instance, nullptr);
    }

    WWDEBUG_SAY(("VkWrapper::Shutdown - Vulkan backend shut down\n"));
}

bool VkWrapper::CreateInstance()
{
    // TODO: Load Vulkan library dynamically and get function pointers
    // For now, assume Vulkan is linked

    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "C&C Generals Zero Hour";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "WW3D Vulkan";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    // Get required extensions
    std::vector<const char*> extensions;
    extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
#ifdef _WIN32
    extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#else
    extensions.push_back(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
#endif

    if (enableValidationLayers) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    if (enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    } else {
        createInfo.enabledLayerCount = 0;
    }

    VkResult result = vkCreateInstance(&createInfo, nullptr, &context.instance);
    if (result != VK_SUCCESS) {
        WWDEBUG_SAY(("vkCreateInstance failed with error code: %d\n", result));
        return false;
    }

    return true;
}

// ... (continued in next message due to length)
