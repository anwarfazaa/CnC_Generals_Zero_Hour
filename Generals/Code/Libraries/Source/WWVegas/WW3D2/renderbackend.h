/*
**	Command & Conquer Generals(tm)
**	Copyright 2025 Electronic Arts Inc.
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	Vulkan Backend - Rendering Abstraction Layer
**	This provides a common interface for both DirectX 8 and Vulkan backends
*/

#ifndef RENDER_BACKEND_H
#define RENDER_BACKEND_H

#include "always.h"
#include "matrix4.h"
#include "vector3.h"
#include "vector4.h"

// Forward declarations
class VertexMaterialClass;
class ShaderClass;
class TextureClass;
class VertexBufferClass;
class IndexBufferClass;
class LightEnvironmentClass;

enum RenderBackendType {
    BACKEND_DX8,
    BACKEND_VULKAN
};

// Common light structure (maps to both D3DLIGHT8 and Vulkan)
struct RenderLight {
    int type;           // 1=point, 2=spot, 3=directional
    Vector4 diffuse;
    Vector4 ambient;
    Vector4 specular;
    Vector3 position;
    Vector3 direction;
    float range;
    float falloff;
    float attenuation0;
    float attenuation1;
    float attenuation2;
    float theta;
    float phi;
};

// Viewport structure
struct RenderViewport {
    unsigned x;
    unsigned y;
    unsigned width;
    unsigned height;
    float minZ;
    float maxZ;
};

/**
 * RenderBackend - Abstract interface for rendering backends
 *
 * This interface abstracts the rendering API, allowing both DX8 and Vulkan
 * implementations to coexist and be selected at runtime.
 */
class RenderBackend
{
public:
    virtual ~RenderBackend() {}

    // Initialization
    virtual bool Init(void* hwnd) = 0;
    virtual void Shutdown() = 0;
    virtual RenderBackendType GetType() const = 0;

    // Device management
    virtual bool CreateDevice() = 0;
    virtual void ReleaseDevice() = 0;
    virtual bool ResetDevice(bool reload_assets = true) = 0;

    // Frame control
    virtual void BeginScene() = 0;
    virtual void EndScene(bool flip_frame = true) = 0;
    virtual void Clear(bool clear_color, bool clear_z_stencil,
                      const Vector3& color, float dest_alpha = 0.0f,
                      float z = 1.0f, unsigned stencil = 0) = 0;

    // State management
    virtual void SetViewport(const RenderViewport& viewport) = 0;
    virtual void SetTransform(int transform_type, const Matrix4& m) = 0;
    virtual void GetTransform(int transform_type, Matrix4& m) = 0;

    // Lighting
    virtual void SetLight(unsigned index, const RenderLight* light) = 0;
    virtual void EnableLight(unsigned index, bool enable) = 0;
    virtual void SetLightEnvironment(LightEnvironmentClass* light_env) = 0;

    // Fog
    virtual void SetFog(bool enable, const Vector3& color, float start, float end) = 0;

    // Materials and shaders
    virtual void SetMaterial(const VertexMaterialClass* material) = 0;
    virtual void SetShader(const ShaderClass& shader) = 0;

    // Textures
    virtual void SetTexture(unsigned stage, TextureClass* texture) = 0;

    // Buffers
    virtual void SetVertexBuffer(const VertexBufferClass* vb) = 0;
    virtual void SetIndexBuffer(const IndexBufferClass* ib, unsigned short index_base_offset) = 0;

    // Drawing
    virtual void DrawIndexedPrimitive(unsigned primitive_type,
                                     unsigned short start_index,
                                     unsigned short polygon_count,
                                     unsigned short min_vertex_index,
                                     unsigned short vertex_count) = 0;

    // Resource creation (return opaque handles)
    virtual void* CreateTexture(unsigned width, unsigned height,
                               unsigned format, unsigned mip_levels) = 0;
    virtual void* CreateVertexBuffer(unsigned size, unsigned format) = 0;
    virtual void* CreateIndexBuffer(unsigned size) = 0;

    virtual void DestroyTexture(void* texture) = 0;
    virtual void DestroyVertexBuffer(void* buffer) = 0;
    virtual void DestroyIndexBuffer(void* buffer) = 0;

    // Statistics
    virtual unsigned GetLastFrameDX8Calls() = 0;
    virtual void ResetStatistics() = 0;
};

/**
 * RenderBackendManager - Singleton to manage backend selection
 */
class RenderBackendManager
{
public:
    static RenderBackendManager& Instance();

    void SetBackend(RenderBackendType type);
    RenderBackend* GetBackend();
    RenderBackendType GetCurrentBackendType() const { return current_backend_type; }

private:
    RenderBackendManager();
    ~RenderBackendManager();
    RenderBackendManager(const RenderBackendManager&) = delete;
    RenderBackendManager& operator=(const RenderBackendManager&) = delete;

    void CreateBackend(RenderBackendType type);
    void DestroyBackend();

    RenderBackend* current_backend;
    RenderBackendType current_backend_type;
};

#endif // RENDER_BACKEND_H
