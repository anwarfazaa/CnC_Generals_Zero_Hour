@echo off
REM ============================================================================
REM Command & Conquer Generals Zero Hour - Windows Build Script
REM ============================================================================
REM This script builds the game on Windows with Visual Studio or other
REM compatible compilers.
REM
REM Usage:
REM   build-windows.bat [Release|Debug] [options]
REM
REM Options:
REM   Release         - Build in Release mode (default)
REM   Debug           - Build in Debug mode
REM   --clean         - Clean before building
REM   --no-vulkan     - Disable Vulkan rendering (use DirectX 8)
REM   --no-60fps      - Disable 60 FPS (use legacy 30 FPS)
REM   --no-ultrawide  - Disable ultra-wide display support
REM   --help          - Show this help message
REM
REM ============================================================================

setlocal enabledelayedexpansion

REM Default configuration
set BUILD_TYPE=Release
set CLEAN_BUILD=0
set USE_VULKAN=ON
set ENABLE_60FPS=ON
set ENABLE_ULTRAWIDE=ON
set ENABLE_UNCAPPED_FPS=ON
set SHOW_HELP=0

REM Parse command line arguments
:parse_args
if "%~1"=="" goto args_done
if /i "%~1"=="Debug" (
    set BUILD_TYPE=Debug
    shift
    goto parse_args
)
if /i "%~1"=="Release" (
    set BUILD_TYPE=Release
    shift
    goto parse_args
)
if /i "%~1"=="--clean" (
    set CLEAN_BUILD=1
    shift
    goto parse_args
)
if /i "%~1"=="--no-vulkan" (
    set USE_VULKAN=OFF
    shift
    goto parse_args
)
if /i "%~1"=="--no-60fps" (
    set ENABLE_60FPS=OFF
    shift
    goto parse_args
)
if /i "%~1"=="--no-ultrawide" (
    set ENABLE_ULTRAWIDE=OFF
    shift
    goto parse_args
)
if /i "%~1"=="--help" (
    set SHOW_HELP=1
    shift
    goto parse_args
)
echo Unknown option: %~1
echo Run 'build-windows.bat --help' for usage information
exit /b 1
:args_done

if %SHOW_HELP%==1 (
    echo.
    echo Command ^& Conquer Generals Zero Hour - Windows Build Script
    echo.
    echo Usage:
    echo   build-windows.bat [Release^|Debug] [options]
    echo.
    echo Build Types:
    echo   Release         Build in Release mode ^(default^)
    echo   Debug           Build in Debug mode with debug symbols
    echo.
    echo Options:
    echo   --clean         Clean build directory before building
    echo   --no-vulkan     Disable Vulkan rendering ^(use DirectX 8^)
    echo   --no-60fps      Disable 60 FPS mode ^(use legacy 30 FPS^)
    echo   --no-ultrawide  Disable ultra-wide display support
    echo   --help          Show this help message
    echo.
    echo Examples:
    echo   build-windows.bat
    echo   build-windows.bat Debug
    echo   build-windows.bat Release --clean
    echo   build-windows.bat Debug --no-vulkan --no-60fps
    echo.
    exit /b 0
)

echo.
echo ============================================================================
echo  Command ^& Conquer Generals Zero Hour - Windows Build
echo ============================================================================
echo.
echo Build Configuration:
echo   Build Type:       %BUILD_TYPE%
echo   Vulkan:           %USE_VULKAN%
echo   60 FPS:           %ENABLE_60FPS%
echo   Ultra-wide:       %ENABLE_ULTRAWIDE%
echo   Uncapped FPS:     %ENABLE_UNCAPPED_FPS%
echo   Clean Build:      %CLEAN_BUILD%
echo.

REM Check for CMake
where cmake >nul 2>nul
if %errorlevel% neq 0 (
    echo [ERROR] CMake not found in PATH
    echo.
    echo Please install CMake from https://cmake.org/download/
    echo Make sure to add CMake to your system PATH during installation.
    echo.
    exit /b 1
)

REM Get CMake version
for /f "tokens=3" %%i in ('cmake --version ^| findstr /R "[0-9]"') do (
    set CMAKE_VERSION=%%i
    goto cmake_version_done
)
:cmake_version_done
echo [INFO] Found CMake version %CMAKE_VERSION%

REM Check for Visual Studio or compatible compiler
where cl.exe >nul 2>nul
if %errorlevel% neq 0 (
    echo [WARNING] Visual Studio compiler ^(cl.exe^) not found in PATH
    echo.
    echo Attempting to find Visual Studio installation...

    REM Try to find Visual Studio using vswhere
    set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
    if exist "!VSWHERE!" (
        for /f "usebackq tokens=*" %%i in (`"!VSWHERE!" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do (
            set VS_PATH=%%i
        )

        if defined VS_PATH (
            echo [INFO] Found Visual Studio at: !VS_PATH!
            if exist "!VS_PATH!\VC\Auxiliary\Build\vcvarsall.bat" (
                echo [INFO] Setting up Visual Studio environment...
                call "!VS_PATH!\VC\Auxiliary\Build\vcvarsall.bat" x64
            )
        ) else (
            echo [WARNING] Visual Studio not found
            echo.
            echo Please install Visual Studio 2017 or later with C++ development tools
            echo Or run this script from a "Developer Command Prompt for VS"
            echo.
            echo The build will continue but may fail if no suitable compiler is found.
            echo.
            pause
        )
    )
)

REM Create build directory
set BUILD_DIR=build-windows

if %CLEAN_BUILD%==1 (
    if exist "%BUILD_DIR%" (
        echo [INFO] Cleaning build directory...
        rmdir /s /q "%BUILD_DIR%"
    )
)

if not exist "%BUILD_DIR%" (
    echo [INFO] Creating build directory: %BUILD_DIR%
    mkdir "%BUILD_DIR%"
)

cd "%BUILD_DIR%"

REM Configure with CMake
echo.
echo ============================================================================
echo  Configuring with CMake...
echo ============================================================================
echo.

cmake .. ^
    -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ^
    -DUSE_VULKAN=%USE_VULKAN% ^
    -DENABLE_60FPS=%ENABLE_60FPS% ^
    -DENABLE_ULTRAWIDE=%ENABLE_ULTRAWIDE% ^
    -DENABLE_UNCAPPED_FPS=%ENABLE_UNCAPPED_FPS% ^
    -DPERMISSIVE_BUILD=ON

if %errorlevel% neq 0 (
    echo.
    echo [ERROR] CMake configuration failed
    cd ..
    exit /b 1
)

REM Build the project
echo.
echo ============================================================================
echo  Building project...
echo ============================================================================
echo.

cmake --build . --config %BUILD_TYPE% -j %NUMBER_OF_PROCESSORS%

if %errorlevel% neq 0 (
    echo.
    echo [ERROR] Build failed
    cd ..
    exit /b 1
)

cd ..

echo.
echo ============================================================================
echo  Build completed successfully!
echo ============================================================================
echo.
echo Build directory: %BUILD_DIR%
echo Build type: %BUILD_TYPE%
echo.

if exist "%BUILD_DIR%\%BUILD_TYPE%\Generals.exe" (
    echo Executable: %BUILD_DIR%\%BUILD_TYPE%\Generals.exe
) else if exist "%BUILD_DIR%\Generals.exe" (
    echo Executable: %BUILD_DIR%\Generals.exe
) else (
    echo [WARNING] Generals.exe not found in expected location
)

echo.
echo To run the game, navigate to the build directory and run Generals.exe
echo.

exit /b 0
