/*
**	Command & Conquer Generals(tm)
**	Copyright 2025 Electronic Arts Inc.
**
**	Vulkan Rendering Backend
**	Modern Vulkan replacement for DirectX 8
*/

#ifndef VK_WRAPPER_H
#define VK_WRAPPER_H

#include "renderbackend.h"
#include "always.h"
#include "matrix4.h"
#include "vector3.h"
#include "vector4.h"
#include "wwstring.h"

// Vulkan headers
#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>

#include <vector>
#include <unordered_map>

// Forward declarations
class VertexMaterialClass;
class ShaderClass;
class TextureClass;
class VertexBufferClass;
class IndexBufferClass;

const unsigned MAX_VK_TEXTURE_STAGES = 4;
const unsigned MAX_VK_LIGHTS = 4;
const unsigned MAX_FRAMES_IN_FLIGHT = 2;

/**
 * VulkanContext - Core Vulkan state
 */
struct VulkanContext {
    // Instance and device
    VkInstance instance;
    VkPhysicalDevice physical_device;
    VkDevice device;
    VkQueue graphics_queue;
    VkQueue present_queue;

    // Queue families
    uint32_t graphics_family_index;
    uint32_t present_family_index;

    // Swapchain
    VkSwapchainKHR swapchain;
    VkFormat swapchain_format;
    VkExtent2D swapchain_extent;
    std::vector<VkImage> swapchain_images;
    std::vector<VkImageView> swapchain_image_views;
    std::vector<VkFramebuffer> swapchain_framebuffers;

    // Render pass
    VkRenderPass render_pass;

    // Command pools and buffers
    VkCommandPool command_pool;
    std::vector<VkCommandBuffer> command_buffers;

    // Synchronization
    std::vector<VkSemaphore> image_available_semaphores;
    std::vector<VkSemaphore> render_finished_semaphores;
    std::vector<VkFence> in_flight_fences;
    std::vector<VkFence> images_in_flight;
    uint32_t current_frame;

    // Depth buffer
    VkImage depth_image;
    VkDeviceMemory depth_image_memory;
    VkImageView depth_image_view;
    VkFormat depth_format;

    // Descriptor pools
    VkDescriptorPool descriptor_pool;
    VkDescriptorSetLayout descriptor_set_layout;

    // Debug
    VkDebugUtilsMessengerEXT debug_messenger;

    // State
    bool in_render_pass;
    bool framebuffer_resized;
    uint32_t current_image_index;

    VulkanContext();
    ~VulkanContext();
};

/**
 * VkBuffer - Vulkan buffer wrapper
 */
struct VkBufferWrapper {
    VkBuffer buffer;
    VkDeviceMemory memory;
    VkDeviceSize size;
    void* mapped_data;

    VkBufferWrapper();
    ~VkBufferWrapper();
};

/**
 * VkTexture - Vulkan texture wrapper
 */
struct VkTextureWrapper {
    VkImage image;
    VkDeviceMemory memory;
    VkImageView image_view;
    VkSampler sampler;
    uint32_t width;
    uint32_t height;
    uint32_t mip_levels;
    VkFormat format;

    VkTextureWrapper();
    ~VkTextureWrapper();
};

/**
 * VulkanPipeline - Graphics pipeline state
 */
struct VulkanPipeline {
    VkPipeline pipeline;
    VkPipelineLayout layout;

    // State hash for caching
    uint64_t state_hash;

    VulkanPipeline();
    ~VulkanPipeline();
};

/**
 * VulkanUniformBuffer - Per-frame uniform data
 */
struct VulkanUniformBuffer {
    Matrix4 world;
    Matrix4 view;
    Matrix4 projection;
    RenderLight lights[MAX_VK_LIGHTS];
    Vector4 fog_color;
    float fog_start;
    float fog_end;
    int fog_enabled;
    int light_enables[MAX_VK_LIGHTS];
};

/**
 * VkWrapper - Vulkan rendering backend implementation
 */
class VkWrapper : public RenderBackend
{
public:
    VkWrapper();
    virtual ~VkWrapper();

    // RenderBackend interface
    virtual bool Init(void* hwnd) override;
    virtual void Shutdown() override;
    virtual RenderBackendType GetType() const override { return BACKEND_VULKAN; }

    virtual bool CreateDevice() override;
    virtual void ReleaseDevice() override;
    virtual bool ResetDevice(bool reload_assets = true) override;

    virtual void BeginScene() override;
    virtual void EndScene(bool flip_frame = true) override;
    virtual void Clear(bool clear_color, bool clear_z_stencil,
                      const Vector3& color, float dest_alpha = 0.0f,
                      float z = 1.0f, unsigned stencil = 0) override;

    virtual void SetViewport(const RenderViewport& viewport) override;
    virtual void SetTransform(int transform_type, const Matrix4& m) override;
    virtual void GetTransform(int transform_type, Matrix4& m) override;

    virtual void SetLight(unsigned index, const RenderLight* light) override;
    virtual void EnableLight(unsigned index, bool enable) override;
    virtual void SetLightEnvironment(LightEnvironmentClass* light_env) override;

    virtual void SetFog(bool enable, const Vector3& color, float start, float end) override;

    virtual void SetMaterial(const VertexMaterialClass* material) override;
    virtual void SetShader(const ShaderClass& shader) override;

    virtual void SetTexture(unsigned stage, TextureClass* texture) override;

    virtual void SetVertexBuffer(const VertexBufferClass* vb) override;
    virtual void SetIndexBuffer(const IndexBufferClass* ib, unsigned short index_base_offset) override;

    virtual void DrawIndexedPrimitive(unsigned primitive_type,
                                     unsigned short start_index,
                                     unsigned short polygon_count,
                                     unsigned short min_vertex_index,
                                     unsigned short vertex_count) override;

    virtual void* CreateTexture(unsigned width, unsigned height,
                               unsigned format, unsigned mip_levels) override;
    virtual void* CreateVertexBuffer(unsigned size, unsigned format) override;
    virtual void* CreateIndexBuffer(unsigned size) override;

    virtual void DestroyTexture(void* texture) override;
    virtual void DestroyVertexBuffer(void* buffer) override;
    virtual void DestroyIndexBuffer(void* buffer) override;

    virtual unsigned GetLastFrameDX8Calls() override;
    virtual void ResetStatistics() override;

private:
    // Initialization helpers
    bool CreateInstance();
    bool SetupDebugMessenger();
    bool PickPhysicalDevice();
    bool CreateLogicalDevice();
    bool CreateSwapchain();
    bool CreateImageViews();
    bool CreateRenderPass();
    bool CreateDescriptorSetLayout();
    bool CreateGraphicsPipeline();
    bool CreateFramebuffers();
    bool CreateCommandPool();
    bool CreateDepthResources();
    bool CreateUniformBuffers();
    bool CreateDescriptorPool();
    bool CreateDescriptorSets();
    bool CreateCommandBuffers();
    bool CreateSyncObjects();

    // Cleanup helpers
    void CleanupSwapchain();

    // Helper functions
    uint32_t FindMemoryType(uint32_t type_filter, VkMemoryPropertyFlags properties);
    VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates,
                                 VkImageTiling tiling,
                                 VkFormatFeatureFlags features);
    VkFormat FindDepthFormat();

    bool CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
                     VkMemoryPropertyFlags properties,
                     VkBuffer& buffer, VkDeviceMemory& buffer_memory);

    void CopyBuffer(VkBuffer src_buffer, VkBuffer dst_buffer, VkDeviceSize size);

    bool CreateImage(uint32_t width, uint32_t height, uint32_t mip_levels,
                    VkFormat format, VkImageTiling tiling,
                    VkImageUsageFlags usage, VkMemoryPropertyFlags properties,
                    VkImage& image, VkDeviceMemory& image_memory);

    VkImageView CreateImageView(VkImage image, VkFormat format,
                                VkImageAspectFlags aspect_flags,
                                uint32_t mip_levels);

    void TransitionImageLayout(VkImage image, VkFormat format,
                              VkImageLayout old_layout, VkImageLayout new_layout,
                              uint32_t mip_levels);

    VulkanPipeline* GetOrCreatePipeline();
    uint64_t ComputePipelineHash();

    void UpdateUniformBuffer(uint32_t current_image);

    // State
    VulkanContext context;
    void* window_handle;

    // Current render state
    Matrix4 current_world;
    Matrix4 current_view;
    Matrix4 current_projection;
    RenderLight current_lights[MAX_VK_LIGHTS];
    bool current_light_enables[MAX_VK_LIGHTS];
    Vector3 fog_color;
    float fog_start;
    float fog_end;
    bool fog_enabled;

    RenderViewport current_viewport;

    // Current bindings
    VkBufferWrapper* current_vertex_buffer;
    VkBufferWrapper* current_index_buffer;
    VkTextureWrapper* current_textures[MAX_VK_TEXTURE_STAGES];

    // Uniform buffers (one per frame in flight)
    std::vector<VkBuffer> uniform_buffers;
    std::vector<VkDeviceMemory> uniform_buffers_memory;
    std::vector<VkDescriptorSet> descriptor_sets;

    // Pipeline cache
    std::unordered_map<uint64_t, VulkanPipeline*> pipeline_cache;
    VulkanPipeline* current_pipeline;

    // Statistics
    unsigned draw_calls;
    unsigned last_frame_draw_calls;

    // Window surface
    VkSurfaceKHR surface;

    // Resolution
    int resolution_width;
    int resolution_height;
};

#endif // VK_WRAPPER_H
