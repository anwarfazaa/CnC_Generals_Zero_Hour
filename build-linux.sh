#!/bin/bash
# ============================================================================
# Command & Conquer Generals Zero Hour - Linux Build Script
# ============================================================================
# This script builds the game on Linux with GCC or Clang.
#
# Usage:
#   ./build-linux.sh [Release|Debug] [options]
#
# Options:
#   Release         - Build in Release mode (default)
#   Debug           - Build in Debug mode
#   --clean         - Clean before building
#   --no-vulkan     - Disable Vulkan rendering
#   --no-60fps      - Disable 60 FPS (use legacy 30 FPS)
#   --no-ultrawide  - Disable ultra-wide display support
#   --jobs N        - Number of parallel jobs (default: CPU cores)
#   --help          - Show this help message
#
# ============================================================================

set -e  # Exit on error

# Default configuration
BUILD_TYPE="Release"
CLEAN_BUILD=0
USE_VULKAN="ON"
ENABLE_60FPS="ON"
ENABLE_ULTRAWIDE="ON"
ENABLE_UNCAPPED_FPS="ON"
SHOW_HELP=0
JOBS=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Helper functions
print_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        Release)
            BUILD_TYPE="Release"
            shift
            ;;
        Debug)
            BUILD_TYPE="Debug"
            shift
            ;;
        --clean)
            CLEAN_BUILD=1
            shift
            ;;
        --no-vulkan)
            USE_VULKAN="OFF"
            shift
            ;;
        --no-60fps)
            ENABLE_60FPS="OFF"
            shift
            ;;
        --no-ultrawide)
            ENABLE_ULTRAWIDE="OFF"
            shift
            ;;
        --jobs)
            JOBS="$2"
            shift 2
            ;;
        --help)
            SHOW_HELP=1
            shift
            ;;
        *)
            print_error "Unknown option: $1"
            echo "Run './build-linux.sh --help' for usage information"
            exit 1
            ;;
    esac
done

if [[ $SHOW_HELP -eq 1 ]]; then
    cat << EOF

Command & Conquer Generals Zero Hour - Linux Build Script

Usage:
  ./build-linux.sh [Release|Debug] [options]

Build Types:
  Release         Build in Release mode (default)
  Debug           Build in Debug mode with debug symbols

Options:
  --clean         Clean build directory before building
  --no-vulkan     Disable Vulkan rendering
  --no-60fps      Disable 60 FPS mode (use legacy 30 FPS)
  --no-ultrawide  Disable ultra-wide display support
  --jobs N        Number of parallel build jobs (default: CPU cores)
  --help          Show this help message

Examples:
  ./build-linux.sh
  ./build-linux.sh Debug
  ./build-linux.sh Release --clean
  ./build-linux.sh Debug --no-vulkan --no-60fps
  ./build-linux.sh Release --jobs 8

EOF
    exit 0
fi

echo ""
echo "============================================================================"
echo "  Command & Conquer Generals Zero Hour - Linux Build"
echo "============================================================================"
echo ""
echo "Build Configuration:"
echo "  Build Type:       $BUILD_TYPE"
echo "  Vulkan:           $USE_VULKAN"
echo "  60 FPS:           $ENABLE_60FPS"
echo "  Ultra-wide:       $ENABLE_ULTRAWIDE"
echo "  Uncapped FPS:     $ENABLE_UNCAPPED_FPS"
echo "  Clean Build:      $CLEAN_BUILD"
echo "  Parallel Jobs:    $JOBS"
echo ""

# Check for required tools
print_info "Checking for required build tools..."

# Check for CMake
if ! command -v cmake &> /dev/null; then
    print_error "CMake not found"
    echo ""
    echo "Please install CMake:"
    echo "  Ubuntu/Debian:  sudo apt install cmake"
    echo "  Fedora/RHEL:    sudo dnf install cmake"
    echo "  Arch Linux:     sudo pacman -S cmake"
    echo ""
    exit 1
fi

CMAKE_VERSION=$(cmake --version | head -n1 | awk '{print $3}')
print_info "Found CMake version $CMAKE_VERSION"

# Check for C++ compiler
if ! command -v g++ &> /dev/null && ! command -v clang++ &> /dev/null; then
    print_error "No C++ compiler found (g++ or clang++)"
    echo ""
    echo "Please install a C++ compiler:"
    echo "  Ubuntu/Debian:  sudo apt install build-essential"
    echo "  Fedora/RHEL:    sudo dnf groupinstall 'Development Tools'"
    echo "  Arch Linux:     sudo pacman -S base-devel"
    echo ""
    exit 1
fi

if command -v g++ &> /dev/null; then
    CXX_COMPILER="g++"
    CXX_VERSION=$(g++ --version | head -n1 | awk '{print $3}')
elif command -v clang++ &> /dev/null; then
    CXX_COMPILER="clang++"
    CXX_VERSION=$(clang++ --version | head -n1 | awk '{print $3}')
fi
print_info "Found C++ compiler: $CXX_COMPILER $CXX_VERSION"

# Check for Python3 (needed for shader compilation)
if ! command -v python3 &> /dev/null; then
    print_warning "Python3 not found - shader compilation may fail"
    echo "  Install with: sudo apt install python3"
else
    PYTHON_VERSION=$(python3 --version | awk '{print $2}')
    print_info "Found Python $PYTHON_VERSION"
fi

# Check for Vulkan SDK (if enabled)
if [[ "$USE_VULKAN" == "ON" ]]; then
    print_info "Checking for Vulkan SDK..."

    if [[ -n "$VULKAN_SDK" ]]; then
        print_info "Vulkan SDK found at: $VULKAN_SDK"
    elif pkg-config --exists vulkan 2>/dev/null; then
        VULKAN_VERSION=$(pkg-config --modversion vulkan)
        print_info "Vulkan found via pkg-config (version $VULKAN_VERSION)"
    else
        print_warning "Vulkan SDK not found"
        echo ""
        echo "To install Vulkan SDK:"
        echo "  Ubuntu/Debian:"
        echo "    sudo apt install libvulkan-dev vulkan-tools"
        echo "  Fedora/RHEL:"
        echo "    sudo dnf install vulkan-headers vulkan-loader-devel"
        echo "  Arch Linux:"
        echo "    sudo pacman -S vulkan-headers vulkan-icd-loader"
        echo ""
        echo "Or download from: https://vulkan.lunarg.com/"
        echo ""
        echo "Build will continue but may fail if Vulkan is required."
        echo ""
        read -p "Press Enter to continue or Ctrl+C to abort..."
    fi

    # Check for GLSL compiler
    if command -v glslc &> /dev/null; then
        print_info "Found glslc shader compiler"
    elif command -v glslangValidator &> /dev/null; then
        print_info "Found glslangValidator shader compiler"
    else
        print_warning "No GLSL shader compiler found (glslc or glslangValidator)"
        echo "  Install with: sudo apt install glslang-tools"
    fi
fi

# Check for additional dependencies
print_info "Checking for additional dependencies..."

MISSING_DEPS=""

# Check for X11 development files
if ! pkg-config --exists x11 2>/dev/null; then
    MISSING_DEPS="$MISSING_DEPS libx11-dev"
fi

# Check for OpenGL
if ! pkg-config --exists gl 2>/dev/null; then
    MISSING_DEPS="$MISSING_DEPS libgl1-mesa-dev"
fi

if [[ -n "$MISSING_DEPS" ]]; then
    print_warning "Some optional dependencies are missing:"
    echo "  Missing: $MISSING_DEPS"
    echo "  Install with: sudo apt install $MISSING_DEPS"
    echo ""
fi

# Create build directory
BUILD_DIR="build-linux"

if [[ $CLEAN_BUILD -eq 1 ]]; then
    if [[ -d "$BUILD_DIR" ]]; then
        print_info "Cleaning build directory..."
        rm -rf "$BUILD_DIR"
    fi
fi

if [[ ! -d "$BUILD_DIR" ]]; then
    print_info "Creating build directory: $BUILD_DIR"
    mkdir -p "$BUILD_DIR"
fi

cd "$BUILD_DIR"

# Configure with CMake
echo ""
echo "============================================================================"
echo "  Configuring with CMake..."
echo "============================================================================"
echo ""

cmake .. \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
    -DUSE_VULKAN="$USE_VULKAN" \
    -DENABLE_60FPS="$ENABLE_60FPS" \
    -DENABLE_ULTRAWIDE="$ENABLE_ULTRAWIDE" \
    -DENABLE_UNCAPPED_FPS="$ENABLE_UNCAPPED_FPS" \
    -DPERMISSIVE_BUILD=ON

if [[ $? -ne 0 ]]; then
    print_error "CMake configuration failed"
    cd ..
    exit 1
fi

# Build the project
echo ""
echo "============================================================================"
echo "  Building project..."
echo "============================================================================"
echo ""

cmake --build . --config "$BUILD_TYPE" -j "$JOBS"

if [[ $? -ne 0 ]]; then
    print_error "Build failed"
    cd ..
    exit 1
fi

cd ..

# Success message
echo ""
echo "============================================================================"
print_success "Build completed successfully!"
echo "============================================================================"
echo ""
echo "Build directory: $BUILD_DIR"
echo "Build type: $BUILD_TYPE"
echo ""

# Find executable
EXECUTABLE=""
if [[ -f "$BUILD_DIR/Generals" ]]; then
    EXECUTABLE="$BUILD_DIR/Generals"
elif [[ -f "$BUILD_DIR/bin/Generals" ]]; then
    EXECUTABLE="$BUILD_DIR/bin/Generals"
elif [[ -f "Run/Generals" ]]; then
    EXECUTABLE="Run/Generals"
fi

if [[ -n "$EXECUTABLE" ]]; then
    echo "Executable: $EXECUTABLE"
    echo ""
    echo "To run the game:"
    echo "  cd $(dirname "$EXECUTABLE")"
    echo "  ./$(basename "$EXECUTABLE")"
else
    print_warning "Generals executable not found in expected location"
fi

echo ""

exit 0
