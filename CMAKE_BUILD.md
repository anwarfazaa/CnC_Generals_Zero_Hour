# CMake Build Instructions for C&C Generals

This document describes how to build Command & Conquer Generals and Zero Hour using CMake with modern compilers.

## Prerequisites

### Required Tools
- CMake 3.15 or higher
- A C++17-compatible compiler:
  - **Windows**: Visual Studio 2017 or later, OR MinGW-w64
  - **Linux**: GCC 7+ or Clang 5+

### Required Dependencies

As documented in the main README.md, you will need to obtain the following dependencies:

1. **DirectX SDK (Version 9.0 or higher)** - Place in `Generals/Code/Libraries/DirectX/`
2. **STLport 4.5.3** - Place in `Generals/Code/Libraries/STLport-4.5.3/`
   - Apply the `stlport.diff` patch as described in the main README
3. **Additional libraries** (as listed in README.md):
   - 3DSMax 4 SDK (for tools only)
   - NVASM
   - BYTEmark
   - RAD Miles Sound System SDK
   - RAD Bink SDK
   - SafeDisk API
   - GameSpy SDK
   - ZLib 1.1.4
   - LZH-Light 1.0

**Note**: The build system will warn about missing dependencies but will attempt to continue in permissive mode. The build may fail during linking if critical dependencies are missing.

## Building on Windows

### Using Visual Studio

```cmd
# Create a build directory
mkdir build
cd build

# Generate Visual Studio project files
cmake .. -G "Visual Studio 16 2019" -A Win32

# Build
cmake --build . --config Release

# Or open the generated solution file in Visual Studio
start CnC_Generals.sln
```

### Using MinGW

```cmd
# Create a build directory
mkdir build
cd build

# Generate Makefiles
cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build .
```

## Building on Linux (Experimental)

**Warning**: This game was designed for Windows. Linux builds are experimental and will require significant additional work, especially for DirectX dependencies.

```bash
# Create a build directory
mkdir build
cd build

# Generate Makefiles
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build . -j$(nproc)
```

For Linux builds, you may want to use Wine or implement platform-specific rendering backends.

## Build Options

You can customize the build using CMake options:

```bash
cmake .. -DBUILD_GENERALS=ON \
         -DBUILD_GENERALSZH=ON \
         -DBUILD_TOOLS=OFF \
         -DPERMISSIVE_BUILD=ON \
         -DCMAKE_BUILD_TYPE=Release
```

### Available Options

- `BUILD_GENERALS` (ON by default) - Build C&C Generals
- `BUILD_GENERALSZH` (ON by default) - Build C&C Generals Zero Hour
- `BUILD_TOOLS` (OFF by default) - Build development tools
- `PERMISSIVE_BUILD` (ON by default) - Continue building despite missing dependencies

### Build Types

- `Release` - Optimized build (defines NDEBUG, _RELEASE)
- `Debug` - Debug build with symbols (defines _DEBUG, DEBUG)
- `RelWithDebInfo` - Release with debug info
- `MinSizeRel` - Size-optimized release

## Common Issues and Solutions

### Issue: "Missing DirectX SDK"

**Solution**: Download the DirectX SDK (June 2010 is recommended) and extract it to `Generals/Code/Libraries/DirectX/`. Ensure the structure is:
```
Generals/Code/Libraries/DirectX/
  ├── Include/
  └── Lib/
```

### Issue: "STLport not found"

**Solution**:
1. Download STLport 4.5.3 from SourceForge or other archives
2. Extract to `Generals/Code/Libraries/STLport-4.5.3/`
3. Apply the provided patch: `patch -p0 < stlport.diff`

### Issue: Compilation errors with modern C++

**Solution**: The `cmake_compat.h` header provides compatibility shims for modern compilers. If you encounter specific errors:
1. Check if they're related to deprecated C++ features
2. Update the compatibility header or CMake flags
3. Consider using `-fpermissive` flag (already enabled for GCC/Clang)

### Issue: Linking errors for Miles/Bink/GameSpy

**Solution**: These are proprietary SDKs that are not included. You have several options:
1. Obtain the original SDKs (may be difficult)
2. Stub out the functionality
3. Replace with open-source alternatives (e.g., OpenAL for Miles, FFmpeg for Bink)

### Issue: Windows-specific code on Linux

**Solution**: The build system provides basic Windows type definitions for Linux in `cmake_compat.h`. However, significant porting work is needed for:
- DirectX → OpenGL/Vulkan
- Windows API calls → POSIX equivalents
- WinSock → BSD sockets

## Advanced Configuration

### Using a different compiler

```bash
# Use Clang instead of GCC
cmake .. -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_C_COMPILER=clang

# Use specific GCC version
cmake .. -DCMAKE_CXX_COMPILER=g++-11 -DCMAKE_C_COMPILER=gcc-11
```

### Verbose build output

```bash
cmake --build . --verbose
# or
make VERBOSE=1
```

### Clean build

```bash
cmake --build . --target clean
# or
make clean
```

### Install

```bash
cmake --build . --target install
# or
make install
```

## Project Structure

The CMake build system is organized as follows:

```
CnC_Generals_Zero_Hour/
├── CMakeLists.txt                    # Root build configuration
├── CMAKE_BUILD.md                    # This file
├── Generals/
│   └── Code/
│       ├── CMakeLists.txt           # Generals-specific build config
│       ├── cmake_compat.h           # Compatibility header
│       ├── GameEngine/              # Game engine library
│       ├── GameEngineDevice/        # Platform-specific device code
│       ├── Libraries/               # Third-party and WW libraries
│       └── Main/                    # Entry point
└── GeneralsMD/                      # Zero Hour expansion
    └── Code/
        └── CMakeLists.txt           # (To be created)
```

## Contributing

If you make improvements to the CMake build system:
1. Test on multiple compilers/platforms if possible
2. Update this documentation
3. Submit changes according to the repository's contribution guidelines

## Troubleshooting

### Enable detailed CMake output

```bash
cmake .. -DCMAKE_VERBOSE_MAKEFILE=ON --trace
```

### Check what's being built

After running CMake, check the output for:
- Found libraries
- Missing dependencies
- Include directories
- Compiler flags

### Report Issues

If you encounter build issues:
1. Check this document first
2. Ensure all dependencies are in the correct locations
3. Try a clean build (`rm -rf build && mkdir build && cd build`)
4. Check the main README.md for additional information

## License

This build system is provided under the same GPL v3 license as the game source code. See LICENSE.md for details.
