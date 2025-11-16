/*
**	Command & Conquer Generals(tm)
**	Copyright 2025 Electronic Arts Inc.
**
**	Render Backend Manager Implementation
*/

#include "renderbackend.h"
#include "wwdebug.h"

#ifdef USE_VULKAN
#include "vkwrapper.h"
#endif

#ifdef USE_DX8
#include "dx8wrapper.h"
#endif

//=============================================================================
// RenderBackendManager - Singleton
//=============================================================================

RenderBackendManager& RenderBackendManager::Instance()
{
    static RenderBackendManager instance;
    return instance;
}

RenderBackendManager::RenderBackendManager()
    : current_backend(nullptr)
    , current_backend_type(BACKEND_VULKAN)  // Default to Vulkan
{
}

RenderBackendManager::~RenderBackendManager()
{
    DestroyBackend();
}

void RenderBackendManager::SetBackend(RenderBackendType type)
{
    if (current_backend && current_backend_type == type) {
        // Already using this backend
        return;
    }

    // Destroy existing backend
    DestroyBackend();

    // Create new backend
    CreateBackend(type);
}

RenderBackend* RenderBackendManager::GetBackend()
{
    if (!current_backend) {
        // Auto-create default backend
#ifdef USE_VULKAN
        CreateBackend(BACKEND_VULKAN);
#elif defined(USE_DX8)
        CreateBackend(BACKEND_DX8);
#else
        WWDEBUG_SAY(("No rendering backend available!\n"));
#endif
    }

    return current_backend;
}

void RenderBackendManager::CreateBackend(RenderBackendType type)
{
    switch (type) {
#ifdef USE_VULKAN
        case BACKEND_VULKAN:
            WWDEBUG_SAY(("Creating Vulkan rendering backend\n"));
            current_backend = new VkWrapper();
            current_backend_type = BACKEND_VULKAN;
            break;
#endif

#ifdef USE_DX8
        case BACKEND_DX8:
            WWDEBUG_SAY(("Creating DirectX 8 rendering backend\n"));
            current_backend = new DX8Wrapper();
            current_backend_type = BACKEND_DX8;
            break;
#endif

        default:
            WWDEBUG_SAY(("Unknown backend type: %d\n", type));
            current_backend = nullptr;
            break;
    }
}

void RenderBackendManager::DestroyBackend()
{
    if (current_backend) {
        current_backend->Shutdown();
        delete current_backend;
        current_backend = nullptr;
    }
}
