# Building Command & Conquer Generals Zero Hour

This document provides comprehensive instructions for building C&C Generals Zero Hour from source on Windows and Linux.

## Quick Start

### Windows
```batch
# Simple build (Release mode, all features enabled)
build-windows.bat

# Debug build
build-windows.bat Debug

# Clean build
build-windows.bat Release --clean
```

### Linux
```bash
# Simple build (Release mode, all features enabled)
./build-linux.sh

# Debug build
./build-linux.sh Debug

# Clean build
./build-linux.sh Release --clean
```

## Table of Contents

- [Prerequisites](#prerequisites)
- [Windows Build](#windows-build)
- [Linux Build](#linux-build)
- [Build Options](#build-options)
- [CMake Options](#cmake-options)
- [Troubleshooting](#troubleshooting)
- [Advanced Building](#advanced-building)

## Prerequisites

### Windows

**Required:**
- **CMake 3.15+** - [Download](https://cmake.org/download/)
- **Visual Studio 2017 or later** with C++ development tools
  - Or Visual Studio 2019/2022 (recommended)
  - Or any CMake-compatible C++ compiler

**Optional:**
- **Vulkan SDK** - [Download](https://vulkan.lunarg.com/) (for Vulkan rendering)
- **Python 3.6+** - For shader compilation
- **Git** - For version control

**Installation:**
```batch
# Install with Chocolatey (package manager for Windows)
choco install cmake visualstudio2022community python git

# Or download installers manually from official websites
```

### Linux

**Required:**
- **CMake 3.15+**
- **GCC 7+ or Clang 6+** (C++17 support)
- **Build essentials** (make, etc.)

**Optional:**
- **Vulkan SDK** - For Vulkan rendering
- **Python 3.6+** - For shader compilation
- **X11 development files** - For windowing
- **OpenGL development files**

**Installation:**

**Ubuntu/Debian:**
```bash
sudo apt update
sudo apt install build-essential cmake git python3

# Optional: Vulkan support
sudo apt install libvulkan-dev vulkan-tools glslang-tools

# Optional: Additional dependencies
sudo apt install libx11-dev libgl1-mesa-dev
```

**Fedora/RHEL:**
```bash
sudo dnf groupinstall 'Development Tools'
sudo dnf install cmake git python3

# Optional: Vulkan support
sudo dnf install vulkan-headers vulkan-loader-devel glslang

# Optional: Additional dependencies
sudo dnf install libX11-devel mesa-libGL-devel
```

**Arch Linux:**
```bash
sudo pacman -S base-devel cmake git python

# Optional: Vulkan support
sudo pacman -S vulkan-headers vulkan-icd-loader glslang

# Optional: Additional dependencies
sudo pacman -S libx11 mesa
```

## Windows Build

### Using Build Script (Recommended)

The build script (`build-windows.bat`) automatically handles configuration and building.

**Basic Usage:**
```batch
# Release build with all features
build-windows.bat

# Debug build
build-windows.bat Debug

# Clean build
build-windows.bat Release --clean

# Show all options
build-windows.bat --help
```

**Advanced Options:**
```batch
# Disable Vulkan (use DirectX 8)
build-windows.bat --no-vulkan

# Disable 60 FPS (use legacy 30 FPS)
build-windows.bat --no-60fps

# Disable ultra-wide support
build-windows.bat --no-ultrawide

# Multiple options
build-windows.bat Release --clean --no-vulkan --no-60fps
```

### Manual CMake Build

If you prefer to use CMake directly:

```batch
# Create build directory
mkdir build-windows
cd build-windows

# Configure (Visual Studio 2022)
cmake .. -G "Visual Studio 17 2022" -A x64 ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DUSE_VULKAN=ON ^
    -DENABLE_60FPS=ON ^
    -DENABLE_ULTRAWIDE=ON

# Build
cmake --build . --config Release -j %NUMBER_OF_PROCESSORS%

# Or open the generated solution in Visual Studio
start CnC_Generals.sln
```

### Output Location

After building, the executable will be located at:
- **Visual Studio:** `build-windows\Release\Generals.exe`
- **Makefiles:** `build-windows\Generals.exe`

## Linux Build

### Using Build Script (Recommended)

The build script (`build-linux.sh`) automatically handles configuration and building.

**Basic Usage:**
```bash
# Make script executable (first time only)
chmod +x build-linux.sh

# Release build with all features
./build-linux.sh

# Debug build
./build-linux.sh Debug

# Clean build
./build-linux.sh Release --clean

# Show all options
./build-linux.sh --help
```

**Advanced Options:**
```bash
# Disable Vulkan
./build-linux.sh --no-vulkan

# Disable 60 FPS (use legacy 30 FPS)
./build-linux.sh --no-60fps

# Disable ultra-wide support
./build-linux.sh --no-ultrawide

# Specify number of build jobs
./build-linux.sh --jobs 8

# Multiple options
./build-linux.sh Release --clean --no-vulkan --no-60fps --jobs 4
```

### Manual CMake Build

If you prefer to use CMake directly:

```bash
# Create build directory
mkdir build-linux
cd build-linux

# Configure
cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DUSE_VULKAN=ON \
    -DENABLE_60FPS=ON \
    -DENABLE_ULTRAWIDE=ON

# Build (using all CPU cores)
cmake --build . -j $(nproc)

# Or use make directly
make -j $(nproc)
```

### Output Location

After building, the executable will be located at:
- `build-linux/Generals`
- Or possibly `Run/Generals`

## Build Options

Both build scripts support the same options:

| Option | Description | Default |
|--------|-------------|---------|
| `Release` | Build in Release mode (optimized) | Yes |
| `Debug` | Build in Debug mode (with symbols) | No |
| `--clean` | Clean build directory before building | No |
| `--no-vulkan` | Disable Vulkan rendering | No (Vulkan enabled) |
| `--no-60fps` | Disable 60 FPS mode | No (60 FPS enabled) |
| `--no-ultrawide` | Disable ultra-wide display support | No (Ultra-wide enabled) |
| `--jobs N` | Number of parallel build jobs (Linux only) | CPU cores |
| `--help` | Show help message | - |

### Build Type Details

**Release Mode:**
- Optimizations enabled (-O3 on GCC/Clang, /O2 on MSVC)
- No debug symbols
- Fastest runtime performance
- Smaller executable size
- **Recommended for playing the game**

**Debug Mode:**
- No optimizations (-O0)
- Full debug symbols (-g)
- Slower runtime performance
- Larger executable size
- **Recommended for development and debugging**

## CMake Options

When using CMake directly, you can specify additional options:

### Core Options

```cmake
-DCMAKE_BUILD_TYPE=Release       # Release or Debug
-DCMAKE_C_COMPILER=gcc           # C compiler
-DCMAKE_CXX_COMPILER=g++         # C++ compiler
```

### Game-Specific Options

```cmake
-DUSE_VULKAN=ON                  # Enable Vulkan rendering (default: ON)
-DUSE_DX8=OFF                    # Enable DirectX 8 (Windows only, default: OFF)
-DENABLE_60FPS=ON                # Enable 60 FPS support (default: ON)
-DENABLE_ULTRAWIDE=ON            # Enable ultra-wide displays (default: ON)
-DENABLE_UNCAPPED_FPS=ON         # Allow uncapped FPS (default: ON)
-DPERMISSIVE_BUILD=ON            # Continue despite missing deps (default: ON)
```

### Build Targets

```cmake
-DBUILD_GENERALS=ON              # Build C&C Generals (default: ON)
-DBUILD_GENERALSZH=ON            # Build Zero Hour (default: ON)
-DBUILD_TOOLS=OFF                # Build development tools (default: OFF)
```

### Example Configurations

**Minimal Build (No Vulkan, 30 FPS):**
```bash
cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DUSE_VULKAN=OFF \
    -DENABLE_60FPS=OFF \
    -DENABLE_ULTRAWIDE=OFF
```

**Maximum Features:**
```bash
cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DUSE_VULKAN=ON \
    -DENABLE_60FPS=ON \
    -DENABLE_ULTRAWIDE=ON \
    -DENABLE_UNCAPPED_FPS=ON
```

**Debug Build for Development:**
```bash
cmake .. \
    -DCMAKE_BUILD_TYPE=Debug \
    -DUSE_VULKAN=ON \
    -DENABLE_60FPS=ON
```

## Troubleshooting

### Windows

**Problem:** "CMake not found"
```
Solution: Install CMake and add it to your PATH
- Download from https://cmake.org/download/
- During installation, select "Add CMake to system PATH"
```

**Problem:** "Visual Studio compiler not found"
```
Solution: Install Visual Studio with C++ development tools
- Download Visual Studio Community (free)
- Select "Desktop development with C++" workload
- Or run build script from "Developer Command Prompt for VS"
```

**Problem:** "Vulkan not found"
```
Solution: Install Vulkan SDK
- Download from https://vulkan.lunarg.com/
- Or build without Vulkan: build-windows.bat --no-vulkan
```

**Problem:** Compilation errors "vkCreateInstance: identifier not found" or "HWND: undeclared identifier"
```
Solution: Vulkan SDK not installed
- These errors indicate the Vulkan SDK headers are missing
- Install Vulkan SDK from https://vulkan.lunarg.com/
- Make sure to restart your command prompt/IDE after installation
- Or build without Vulkan: build-windows.bat --no-vulkan
- CMake will automatically disable Vulkan if SDK is not found
```

**Problem:** "Cannot open include file: 'shaders/basic_vert.spv.h'"
```
Solution: Shader compilation failed
- Requires Vulkan SDK with glslc or glslangValidator
- Install Vulkan SDK completely (not just headers)
- Or build without Vulkan: build-windows.bat --no-vulkan
```

**Problem:** "operator new[] already has a body" (C2084 error)
```
Solution: Fixed in latest version
- This was a compatibility issue with Visual Studio 2022
- Pull latest changes: git pull origin main
- The placement new operators are now conditionally compiled
```

**Problem:** Build fails with linker errors
```
Solution: Missing proprietary dependencies
- Some game assets/libraries are not included in source release
- Build with: -DPERMISSIVE_BUILD=ON (default)
- Or use the build script which enables this automatically
```

### Linux

**Problem:** "CMake not found"
```bash
# Ubuntu/Debian
sudo apt install cmake

# Fedora/RHEL
sudo dnf install cmake

# Arch Linux
sudo pacman -S cmake
```

**Problem:** "No C++ compiler found"
```bash
# Ubuntu/Debian
sudo apt install build-essential

# Fedora/RHEL
sudo dnf groupinstall 'Development Tools'

# Arch Linux
sudo pacman -S base-devel
```

**Problem:** "Vulkan not found"
```bash
# Ubuntu/Debian
sudo apt install libvulkan-dev vulkan-tools

# Fedora/RHEL
sudo dnf install vulkan-headers vulkan-loader-devel

# Arch Linux
sudo pacman -S vulkan-headers vulkan-icd-loader

# Or build without Vulkan
./build-linux.sh --no-vulkan
```

**Problem:** "glslc or glslangValidator not found"
```bash
# Ubuntu/Debian
sudo apt install glslang-tools

# Fedora/RHEL
sudo dnf install glslang

# Arch Linux
sudo pacman -S glslang

# This is only needed for shader compilation
# Pre-compiled shaders may be included
```

**Problem:** "X11 development files not found"
```bash
# Ubuntu/Debian
sudo apt install libx11-dev libgl1-mesa-dev

# Fedora/RHEL
sudo dnf install libX11-devel mesa-libGL-devel

# Arch Linux
sudo pacman -S libx11 mesa
```

**Problem:** Build succeeds but executable doesn't run
```bash
# Check for missing libraries
ldd ./build-linux/Generals

# Install missing dependencies
# Common: libvulkan.so, libGL.so, libX11.so
```

### Common Issues

**Problem:** Out of memory during build
```
Solution: Reduce parallel jobs
- Windows: Task Manager → Performance → See number of cores
- Linux: ./build-linux.sh --jobs 2
```

**Problem:** Build is very slow
```
Solution: Increase parallel jobs
- Windows: Automatically uses all CPU cores
- Linux: ./build-linux.sh --jobs 8 (or number of CPU cores)
```

**Problem:** "Permission denied" on Linux
```bash
# Make build script executable
chmod +x build-linux.sh

# If build directory has permission issues
sudo chown -R $USER:$USER build-linux
```

## Advanced Building

### Cross-Compilation

**Linux to Windows (using MinGW):**
```bash
# Install MinGW
sudo apt install mingw-w64

# Configure for Windows target
cmake .. \
    -DCMAKE_TOOLCHAIN_FILE=cmake/mingw-w64-x86_64.cmake \
    -DCMAKE_BUILD_TYPE=Release
```

### Custom Compiler Flags

**Add extra optimization flags:**
```bash
# GCC/Clang
cmake .. -DCMAKE_CXX_FLAGS="-O3 -march=native"

# MSVC
cmake .. -DCMAKE_CXX_FLAGS="/O2 /GL"
```

### Static Linking

**Build with static libraries:**
```bash
cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_SHARED_LIBS=OFF \
    -DCMAKE_EXE_LINKER_FLAGS="-static"
```

### Building Specific Targets

**Build only specific components:**
```bash
# Build only the main executable
cmake --build . --target Generals

# Build only libraries
cmake --build . --target WW3D2

# Clean specific target
cmake --build . --target clean
```

### Verbose Build Output

**See detailed build commands:**
```bash
# CMake
cmake --build . --verbose

# Make
make VERBOSE=1

# Ninja
ninja -v
```

## Build Script Features

### Windows Script Features
- ✅ Automatic Visual Studio detection
- ✅ Environment setup (vcvarsall.bat)
- ✅ Multi-core parallel building
- ✅ Clean build support
- ✅ Comprehensive error messages
- ✅ Build configuration summary

### Linux Script Features
- ✅ Colored output for easy reading
- ✅ Dependency checking
- ✅ Vulkan SDK detection
- ✅ Automatic parallel jobs (uses all cores)
- ✅ Helpful installation instructions
- ✅ Multiple compiler support (GCC/Clang)

## Performance Tips

### Faster Builds

1. **Use Ninja instead of Make (Linux):**
   ```bash
   sudo apt install ninja-build
   cmake .. -G Ninja
   ninja
   ```

2. **Enable ccache (Linux):**
   ```bash
   sudo apt install ccache
   export CMAKE_CXX_COMPILER_LAUNCHER=ccache
   ```

3. **Use more build jobs:**
   ```bash
   ./build-linux.sh --jobs $(nproc)  # All cores
   ```

4. **Incremental builds:**
   - Don't use `--clean` unless necessary
   - Only changed files will be recompiled

### Smaller Executables

```bash
# Strip debug symbols (Release mode)
strip build-linux/Generals

# Link-time optimization
cmake .. -DCMAKE_INTERPROCEDURAL_OPTIMIZATION=ON
```

## See Also

- [60FPS_ULTRAWIDE.md](60FPS_ULTRAWIDE.md) - Display enhancements documentation
- [VULKAN_INTEGRATION.md](VULKAN_INTEGRATION.md) - Vulkan rendering backend
- [CMAKE_BUILD.md](CMAKE_BUILD.md) - CMake build system details
- [README.md](README.md) - Project overview

## License

This build system is part of the C&C Generals source code and is licensed under GPL v3. See [LICENSE.md](LICENSE.md) for details.
