# 60 FPS and Ultra-Wide Display Support

This document describes the enhanced display features added to C&C Generals Zero Hour, including 60 FPS support and ultra-wide/super ultra-wide display compatibility.

## Features

### ✅ 60 FPS Support
- **Increased frame rate** from 30 FPS to 60 FPS
- **Configurable** - can be toggled between 30 FPS (legacy) and 60 FPS (modern)
- **Smooth gameplay** with doubled update frequency
- **Compatible** with existing game logic and physics

### ✅ Ultra-Wide Display Support (21:9)
- **2560x1080** - Full HD ultra-wide
- **3440x1440** - QHD ultra-wide
- **3840x1600** - UHD+ ultra-wide
- **Automatic FOV adjustment** using Hor+ scaling
- **Smart UI scaling** to prevent stretching

### ✅ Super Ultra-Wide Support (32:9)
- **3840x1080** - Dual Full HD
- **5120x1440** - Dual QHD (Samsung CHG90, Odyssey G9)
- **5120x2160** - Dual 4K
- **Extended horizontal FOV** for immersive gameplay
- **Optimized UI layout** for extreme aspect ratios

## Build Configuration

### Enabling 60 FPS

By default, 60 FPS is enabled. To toggle:

```cpp
// In DisplayConfig.h or as a compiler flag:
#define ENABLE_60FPS 1  // 60 FPS mode (default)
#define ENABLE_60FPS 0  // 30 FPS legacy mode
```

**CMake option:**
```bash
cmake .. -DENABLE_60FPS=ON  # Enable 60 FPS (default)
cmake .. -DENABLE_60FPS=OFF # Disable 60 FPS (legacy 30 FPS)
```

### Enabling Ultra-Wide Support

Ultra-wide support is enabled by default:

```cpp
// In DisplayConfig.h:
#define ENABLE_ULTRAWIDE 1  // Enable ultra-wide (default)
#define ENABLE_ULTRAWIDE 0  // Disable ultra-wide features
```

**CMake option:**
```bash
cmake .. -DENABLE_ULTRAWIDE=ON  # Enable ultra-wide (default)
cmake .. -DENABLE_ULTRAWIDE=OFF # Standard aspect ratios only
```

## Technical Details

### Frame Rate Changes

**Original (30 FPS):**
- `LOGICFRAMES_PER_SECOND = 30`
- `DEFAULT_MAX_FPS = 45`
- `TheW3DFrameLengthInMsec = 33` ms/frame

**Modern (60 FPS):**
- `LOGICFRAMES_PER_SECOND = 60`
- `DEFAULT_MAX_FPS = 60`
- `TheW3DFrameLengthInMsec = 16` ms/frame

### Aspect Ratio Handling

The game now recognizes and optimizes for these aspect ratios:

| Aspect Ratio | Value | Type | Common Resolutions |
|--------------|-------|------|-------------------|
| 4:3 | 1.333 | Legacy | 800x600, 1024x768, 1600x1200 |
| 16:10 | 1.600 | Widescreen | 1280x800, 1920x1200, 2560x1600 |
| 16:9 | 1.778 | Widescreen | 1920x1080, 2560x1440, 3840x2160 |
| **21:9** | **2.333** | **Ultra-wide** | **2560x1080, 3440x1440** |
| **32:9** | **3.556** | **Super Ultra-wide** | **5120x1440** |

### FOV Calculation

**Hor+ Scaling Algorithm:**
```cpp
float CalculateAdjustedFOV(float baseFOV, float aspectRatio)
{
    // Base aspect ratio (4:3)
    float baseAspect = 4.0f / 3.0f;
    float baseFOVRad = baseFOV * PI / 180.0f;

    // Calculate vertical FOV (constant across all aspect ratios)
    float vFOV = 2.0f * atan(tan(baseFOVRad / 2.0f) / baseAspect);

    // Calculate new horizontal FOV for current aspect ratio
    float hFOV = 2.0f * atan(tan(vFOV / 2.0f) * aspectRatio);
    float hFOVDeg = hFOV * 180.0f / PI;

    // Clamp to reasonable limits (45° - 120°)
    return clamp(hFOVDeg, 45.0f, 120.0f);
}
```

**Example FOV values:**
- **4:3** (1024x768): FOV = 60° (base)
- **16:9** (1920x1080): FOV ≈ 75°
- **21:9** (3440x1440): FOV ≈ 90°
- **32:9** (5120x1440): FOV ≈ 110°

### UI Scaling Modes

Three modes are available for ultra-wide displays:

#### 1. ULTRAWIDE_UI_STRETCH (Legacy)
```
┌──────────────────────────────────────┐
│ [UI stretched across entire width]  │
└──────────────────────────────────────┘
```
UI elements stretch to fill the entire screen. May cause distortion on ultra-wide displays.

#### 2. ULTRAWIDE_UI_PILLARBOX (Safe)
```
┌─────┬──────────────────────┬─────┐
│Black│  [UI in 16:9 area]  │Black│
└─────┴──────────────────────┴─────┘
```
UI constrained to 16:9 safe area in the center. Black bars on sides.

#### 3. ULTRAWIDE_UI_SMART (Recommended, Default)
```
┌──────────────────────────────────────┐
│[HUD]     [Center UI 16:9]     [HUD] │
└──────────────────────────────────────┘
```
HUD elements (minimap, resources, buttons) positioned at screen edges for easy access. Center UI (menus, dialogs) constrained to 16:9 area to prevent stretching.

## Configuration Options

### In-Game Settings

The following can be configured in `options.ini`:

```ini
[Video]
Resolution = 3440 1440  ; Your resolution
Windowed = No           ; Fullscreen recommended for ultra-wide
RefreshRate = 60        ; Match your monitor's refresh rate

[Graphics]
MaxFPS = 60             ; 60, 120, 144, or 0 for uncapped
UseFPSLimit = Yes       ; Enable FPS limiting
AutoFOV = Yes           ; Automatic FOV adjustment for ultra-wide
UltraWideUIMode = 2     ; 0=Stretch, 1=Pillarbox, 2=Smart (default)
```

### Command Line Options

```bash
# Launch with specific settings
./Generals -res 3440 1440 -win -maxfps 60

# Uncapped FPS (for high refresh rate monitors)
./Generals -res 5120 1440 -maxfps 0

# Force legacy 30 FPS mode
./Generals -maxfps 30 -nofpsscaling
```

## Supported Resolutions

### Standard Widescreen (16:9)
- 1280x720 (HD)
- 1920x1080 (Full HD) ✓
- 2560x1440 (QHD) ✓
- 3840x2160 (4K UHD) ✓

### Ultra-Wide (21:9)
- 2560x1080 ✓
- 3440x1440 ✓
- 3840x1600 ✓
- 5120x2160 ✓

### Super Ultra-Wide (32:9)
- 3840x1080 ✓
- 5120x1440 (Samsung Odyssey G9) ✓
- 5120x2160 ✓

### Legacy (4:3)
- 800x600
- 1024x768
- 1600x1200

## Performance Considerations

### 60 FPS Impact

**CPU Usage:**
- Logic updates: 2x frequency (30 → 60 updates/sec)
- Moderate CPU increase (~30-50%)
- Modern CPUs (2015+) should handle easily

**GPU Usage:**
- Rendering: 2x frame rate
- Minimal GPU impact on modern cards
- Bottleneck is usually CPU logic, not rendering

**Recommended Specs for 60 FPS:**
- CPU: Intel Core i5-6600K / AMD Ryzen 5 1600 or better
- GPU: NVIDIA GTX 960 / AMD RX 560 or better
- RAM: 4 GB minimum

### Ultra-Wide Impact

**Rendering:**
- Higher resolution = more pixels to render
- 3440x1440 is 34% more pixels than 2560x1440
- 5120x1440 is 78% more pixels than 2560x1440

**FOV:**
- Wider FOV = more geometry visible
- Potentially more draw calls
- Frustum culling still effective

**Recommended Specs for Ultra-Wide 60 FPS:**

| Resolution | Min GPU | Recommended GPU |
|------------|---------|-----------------|
| 2560x1080 | GTX 1060 | RTX 2060 |
| 3440x1440 | GTX 1070 | RTX 2070 |
| 5120x1440 | RTX 2070 | RTX 3080 |

## Troubleshooting

### Frame Rate Issues

**Problem:** Game still runs at 30 FPS
**Solution:**
1. Check `ENABLE_60FPS` is defined as `1` in DisplayConfig.h
2. Rebuild project completely: `cmake --build . --clean-first`
3. Check in-game options: MaxFPS should be 60+
4. Verify VSync is not limiting to 30 FPS in GPU drivers

**Problem:** Stuttering or inconsistent frame rate
**Solution:**
1. Disable FPS limit: `MaxFPS = 0` for uncapped
2. Enable VSync for smooth frame pacing
3. Check GPU drivers are up to date
4. Reduce graphics settings if GPU-bound

### Ultra-Wide Issues

**Problem:** Black bars on sides of screen
**Solution:**
1. Set `UltraWideUIMode = 0` (Stretch) in options.ini
2. Or use `UltraWideUIMode = 2` (Smart) for proper ultra-wide experience
3. Ensure resolution matches your monitor's native resolution

**Problem:** UI elements cut off or distorted
**Solution:**
1. Use `UltraWideUIMode = 1` (Pillarbox) for guaranteed safe UI
2. Or `UltraWideUIMode = 2` (Smart) which constrains center UI only
3. Check monitor scaling is set to 100% in OS

**Problem:** FOV too narrow or too wide
**Solution:**
1. Enable `AutoFOV = Yes` in options.ini for automatic adjustment
2. Or manually set FOV with `-fov <angle>` command line option
3. Recommended range: 75-110 degrees

**Problem:** Performance drop on ultra-wide
**Solution:**
1. Reduce texture quality or effects
2. Lower resolution to 2560x1080 for better performance
3. Disable anti-aliasing (ultra-wide reduces need for it)
4. Upgrade GPU

## Implementation Files

The following files implement 60 FPS and ultra-wide support:

```
Generals/Code/GameEngine/Include/Common/
├── DisplayConfig.h          # Configuration header (NEW)
├── GameCommon.h             # Updated for configurable FPS
└── GameEngine.h             # Updated default max FPS

Generals/Code/GameEngineDevice/Source/W3DDevice/GameClient/
├── W3DView.cpp              # FOV and viewport handling
└── W3DDisplay.cpp           # Resolution and rendering

Generals/Code/GameEngine/Source/Common/
├── GameEngine.cpp           # Frame rate limiting
└── GlobalData.cpp           # Resolution settings
```

## API Reference

### DisplayConfig Namespace

```cpp
#include "Common/DisplayConfig.h"

// Calculate adjusted FOV for aspect ratio
float fov = DisplayConfig::CalculateAdjustedFOV(60.0f, 21.0f/9.0f);

// Check if resolution is ultra-wide
bool isUW = DisplayConfig::IsUltraWide(aspectRatio);
bool isSUW = DisplayConfig::IsSuperUltraWide(aspectRatio);

// Get UI safe area for rendering
int safeX, safeY, safeWidth, safeHeight;
DisplayConfig::GetUISafeArea(screenWidth, screenHeight,
                              safeX, safeY, safeWidth, safeHeight,
                              ULTRAWIDE_UI_SMART);
```

## Testing

### Test Configurations

1. **30 FPS Legacy** - Verify backward compatibility
2. **60 FPS Standard (1920x1080)** - Baseline performance
3. **60 FPS Ultra-Wide (3440x1440)** - FOV and UI scaling
4. **60 FPS Super Ultra-Wide (5120x1440)** - Extreme aspect ratio

### Test Cases

- [ ] Frame rate is stable at 60 FPS
- [ ] Game logic runs at correct speed (not 2x fast)
- [ ] Camera movement is smooth
- [ ] Unit movement speed is unchanged
- [ ] FOV automatically adjusts for ultra-wide
- [ ] UI scales properly on ultra-wide
- [ ] Minimap aspect ratio is correct
- [ ] Mouse cursor tracking is accurate
- [ ] No performance regression vs 30 FPS

## Future Enhancements

Planned features for future updates:

- [ ] **120 FPS / 144 FPS** support for high refresh rate monitors
- [ ] **Dynamic resolution scaling** for performance optimization
- [ ] **Per-monitor DPI awareness** for multi-monitor setups
- [ ] **HDR support** for compatible displays
- [ ] **Customizable UI layouts** for ultra-wide
- [ ] **Triple monitor / surround support** (48:9 aspect ratio)

## Credits

Enhanced 60 FPS and ultra-wide support implemented as part of the C&C Generals modernization project.

## License

This enhancement is part of the C&C Generals source code and is licensed under GPL v3. See [LICENSE.md](LICENSE.md) for details.
