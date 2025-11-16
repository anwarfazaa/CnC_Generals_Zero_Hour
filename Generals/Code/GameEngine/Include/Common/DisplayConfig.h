/*
**	Command & Conquer Generals(tm)
**	Copyright 2025 Electronic Arts Inc.
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	Display Configuration Header
**	Enhanced support for 60 FPS, Ultra-Wide and Super Ultra-Wide displays
*/

#ifndef _DISPLAY_CONFIG_H_
#define _DISPLAY_CONFIG_H_

// ============================================================================
// FRAME RATE CONFIGURATION
// ============================================================================

// Enable 60 FPS support (default: enabled)
// Set to 0 to use legacy 30 FPS mode
#ifndef ENABLE_60FPS
#define ENABLE_60FPS 1
#endif

// Logic frames per second
// Legacy: 30 FPS, Modern: 60 FPS
#if ENABLE_60FPS
#define CONFIG_LOGICFRAMES_PER_SECOND 60
#define CONFIG_DEFAULT_MAX_FPS 60
#else
#define CONFIG_LOGICFRAMES_PER_SECOND 30
#define CONFIG_DEFAULT_MAX_FPS 45
#endif

// Allow uncapped frame rate (vsync off)
#ifndef ENABLE_UNCAPPED_FPS
#define ENABLE_UNCAPPED_FPS 1
#endif

// Maximum FPS when uncapped (prevents excessive CPU usage)
#define CONFIG_MAX_UNCAPPED_FPS 144

// ============================================================================
// ULTRA-WIDE DISPLAY SUPPORT
// ============================================================================

// Enable ultra-wide display support (default: enabled)
#ifndef ENABLE_ULTRAWIDE
#define ENABLE_ULTRAWIDE 1
#endif

// Standard aspect ratios
#define ASPECT_4_3     (4.0f / 3.0f)    // 1.333 - Original game
#define ASPECT_16_9    (16.0f / 9.0f)   // 1.778 - Widescreen
#define ASPECT_16_10   (16.0f / 10.0f)  // 1.600 - Widescreen alternate
#define ASPECT_21_9    (21.0f / 9.0f)   // 2.333 - Ultra-wide
#define ASPECT_32_9    (32.0f / 9.0f)   // 3.556 - Super ultra-wide
#define ASPECT_32_10   (32.0f / 10.0f)  // 3.200 - Super ultra-wide alternate

// Common ultra-wide resolutions
// 21:9 Ultra-wide
#define RES_ULTRAWIDE_2560x1080    2560, 1080
#define RES_ULTRAWIDE_3440x1440    3440, 1440
#define RES_ULTRAWIDE_3840x1600    3840, 1600

// 32:9 Super ultra-wide
#define RES_SUPERWIDE_3840x1080    3840, 1080
#define RES_SUPERWIDE_5120x1440    5120, 1440
#define RES_SUPERWIDE_5120x2160    5120, 2160

// FOV Adjustment for Ultra-Wide
// Automatically adjust horizontal FOV based on aspect ratio
#if ENABLE_ULTRAWIDE
#define CONFIG_AUTO_FOV_ADJUST 1
#else
#define CONFIG_AUTO_FOV_ADJUST 0
#endif

// Base FOV (for 4:3 aspect ratio)
#define CONFIG_BASE_FOV 60.0f

// FOV limits
#define CONFIG_MIN_FOV 45.0f
#define CONFIG_MAX_FOV 120.0f

// ============================================================================
// UI SCALING FOR ULTRA-WIDE
// ============================================================================

// Enable automatic UI scaling for ultra-wide displays
#ifndef ENABLE_ULTRAWIDE_UI_SCALING
#define ENABLE_ULTRAWIDE_UI_SCALING 1
#endif

// UI anchor modes for ultra-wide
enum UltraWideUIMode
{
    ULTRAWIDE_UI_STRETCH,      // Stretch UI across entire screen (legacy)
    ULTRAWIDE_UI_PILLARBOX,    // Keep UI in center with 16:9 safe area
    ULTRAWIDE_UI_SMART,        // HUD elements at edges, center UI in 16:9 area (recommended)
};

// Default UI mode for ultra-wide displays
#define CONFIG_DEFAULT_ULTRAWIDE_UI_MODE ULTRAWIDE_UI_SMART

// Safe area ratio (for ULTRAWIDE_UI_PILLARBOX and ULTRAWIDE_UI_SMART modes)
// UI elements will be constrained to this aspect ratio in the center
#define CONFIG_UI_SAFE_AREA_RATIO ASPECT_16_9

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

#ifdef __cplusplus

namespace DisplayConfig
{
    // Calculate adjusted horizontal FOV for ultra-wide displays
    inline float CalculateAdjustedFOV(float baseFOV, float aspectRatio)
    {
#if CONFIG_AUTO_FOV_ADJUST
        // Use Hor+ scaling for ultra-wide displays
        // Convert vertical FOV to horizontal FOV based on aspect ratio
        float baseAspect = ASPECT_4_3;
        float baseFOVRad = baseFOV * 3.14159265f / 180.0f;

        // Calculate vertical FOV (constant)
        float vFOV = 2.0f * atanf(tanf(baseFOVRad / 2.0f) / baseAspect);

        // Calculate new horizontal FOV for current aspect ratio
        float hFOV = 2.0f * atanf(tanf(vFOV / 2.0f) * aspectRatio);
        float hFOVDeg = hFOV * 180.0f / 3.14159265f;

        // Clamp to reasonable limits
        if (hFOVDeg < CONFIG_MIN_FOV) hFOVDeg = CONFIG_MIN_FOV;
        if (hFOVDeg > CONFIG_MAX_FOV) hFOVDeg = CONFIG_MAX_FOV;

        return hFOVDeg;
#else
        return baseFOV;
#endif
    }

    // Check if aspect ratio is ultra-wide
    inline bool IsUltraWide(float aspectRatio)
    {
        return aspectRatio >= ASPECT_21_9 - 0.01f;
    }

    // Check if aspect ratio is super ultra-wide
    inline bool IsSuperUltraWide(float aspectRatio)
    {
        return aspectRatio >= ASPECT_32_9 - 0.01f;
    }

    // Get UI safe area for ultra-wide displays
    inline void GetUISafeArea(int screenWidth, int screenHeight,
                             int& safeX, int& safeY, int& safeWidth, int& safeHeight,
                             UltraWideUIMode mode = CONFIG_DEFAULT_ULTRAWIDE_UI_MODE)
    {
        float aspectRatio = (float)screenWidth / (float)screenHeight;

        if (mode == ULTRAWIDE_UI_STRETCH || !IsUltraWide(aspectRatio))
        {
            // Full screen
            safeX = 0;
            safeY = 0;
            safeWidth = screenWidth;
            safeHeight = screenHeight;
        }
        else if (mode == ULTRAWIDE_UI_PILLARBOX)
        {
            // Center UI in 16:9 safe area
            safeHeight = screenHeight;
            safeWidth = (int)(safeHeight * CONFIG_UI_SAFE_AREA_RATIO);
            safeX = (screenWidth - safeWidth) / 2;
            safeY = 0;
        }
        else // ULTRAWIDE_UI_SMART
        {
            // Full screen for now (HUD elements can be positioned at edges separately)
            safeX = 0;
            safeY = 0;
            safeWidth = screenWidth;
            safeHeight = screenHeight;
        }
    }
}

#endif // __cplusplus

#endif // _DISPLAY_CONFIG_H_
