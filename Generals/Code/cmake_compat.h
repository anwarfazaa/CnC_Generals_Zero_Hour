/*
**	Command & Conquer Generals(tm)
**	Copyright 2025 Electronic Arts Inc.
**
**	CMake Compatibility Header
**	This file provides compatibility definitions for building with modern compilers
*/

#ifndef _CMAKE_COMPAT_H_
#define _CMAKE_COMPAT_H_

// Handle platform differences
#ifdef _MSC_VER
    // MSVC-specific definitions (already handled in code)
#else
    // GCC/Clang compatibility

    // Disable MSVC-specific pragmas
    #ifndef _MSC_VER
        #define __pragma(x)
        #define __declspec(x)
    #endif

    // Define __int64 for non-MSVC compilers
    #ifndef __int64
        #define __int64 long long
    #endif

    // MSVC calling conventions
    #ifndef __cdecl
        #define __cdecl
    #endif

    #ifndef __stdcall
        #define __stdcall
    #endif

    #ifndef __fastcall
        #define __fastcall
    #endif

    // MSVC-specific keywords
    #ifndef __forceinline
        #define __forceinline inline __attribute__((always_inline))
    #endif

    #ifndef __inline
        #define __inline inline
    #endif

    // Exception specifications (deprecated in C++17)
    #define throw(...)

    // Windows types for Linux builds
    #ifdef _LINUX
        #include <stdint.h>
        #include <cstddef>

        typedef uint32_t DWORD;
        typedef uint16_t WORD;
        typedef uint8_t BYTE;
        typedef int32_t LONG;
        typedef int32_t BOOL;
        typedef void* HANDLE;
        typedef void* HWND;
        typedef void* HINSTANCE;
        typedef void* HBITMAP;
        typedef void* HDC;
        typedef void* HMODULE;
        typedef void* HKEY;
        typedef char* LPSTR;
        typedef const char* LPCSTR;
        typedef wchar_t* LPWSTR;
        typedef const wchar_t* LPCWSTR;
        typedef void* LPVOID;
        typedef const void* LPCVOID;
        typedef uint64_t UINT_PTR;
        typedef int64_t INT_PTR;
        typedef uint64_t ULONG_PTR;
        typedef int64_t LONG_PTR;

        #ifndef TRUE
        #define TRUE 1
        #endif

        #ifndef FALSE
        #define FALSE 0
        #endif

        #ifndef MAX_PATH
        #define MAX_PATH 260
        #endif

        // Stub out Windows-specific functions
        #define OutputDebugString(x)
        #define MessageBox(h, t, c, f) 0
        #define GetLastError() 0
        #define SetLastError(x)
    #endif
#endif

// Compatibility for old STL code
#include <algorithm>
#include <cmath>
#include <cstring>
#include <cstdlib>

// Ensure min/max are available (handle conflicts with macros)
#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

// Re-add min/max as inline functions if needed
#ifndef NOMINMAX
template<typename T>
inline T min(const T& a, const T& b) {
    return (a < b) ? a : b;
}

template<typename T>
inline T max(const T& a, const T& b) {
    return (a > b) ? a : b;
}
#endif

// Handle deprecated C++ features
#ifdef __GNUC__
    #pragma GCC diagnostic ignored "-Wdeprecated"
    #pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

#ifdef __clang__
    #pragma clang diagnostic ignored "-Wdeprecated"
    #pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif

#endif // _CMAKE_COMPAT_H_
