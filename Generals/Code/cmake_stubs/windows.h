/*
**	CMake Windows API Stub Header
**	Provides minimal Windows API definitions for Linux builds
*/

#ifndef _CMAKE_STUB_WINDOWS_H_
#define _CMAKE_STUB_WINDOWS_H_

#ifndef _MSC_VER

#include <stdint.h>
#include <stddef.h>

// Basic Windows types
typedef uint32_t DWORD;
typedef uint32_t UINT;
typedef int32_t LONG;
typedef int32_t INT;
typedef uint16_t WORD;
typedef uint8_t BYTE;
typedef int BOOL;
typedef float FLOAT;
typedef char CHAR;
typedef wchar_t WCHAR;

// Pointer types
typedef void* LPVOID;
typedef const void* LPCVOID;
typedef char* LPSTR;
typedef const char* LPCSTR;
typedef wchar_t* LPWSTR;
typedef const wchar_t* LPCWSTR;
typedef CHAR* LPCH, *PCH;
typedef BYTE* LPBYTE, *PBYTE;
typedef WORD* LPWORD, *PWORD;
typedef DWORD* LPDWORD, *PDWORD;
typedef void* HANDLE;
typedef HANDLE HWND;
typedef HANDLE HINSTANCE;
typedef HANDLE HMODULE;
typedef HANDLE HBITMAP;
typedef HANDLE HDC;
typedef HANDLE HBRUSH;
typedef HANDLE HPEN;
typedef HANDLE HFONT;
typedef HANDLE HICON;
typedef HANDLE HCURSOR;
typedef HANDLE HMENU;
typedef HANDLE HKEY;
typedef HANDLE HGDIOBJ;
typedef HANDLE HPALETTE;
typedef HANDLE HRGN;
typedef HANDLE HACCEL;
typedef HANDLE HGLRC;

// Size types
typedef uint64_t UINT_PTR;
typedef int64_t INT_PTR;
typedef uint64_t ULONG_PTR;
typedef int64_t LONG_PTR;
typedef ULONG_PTR SIZE_T;
typedef LONG_PTR SSIZE_T;
typedef UINT_PTR WPARAM;
typedef LONG_PTR LPARAM;
typedef LONG_PTR LRESULT;

// Constants
#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef NULL
#define NULL 0
#endif

#ifndef MAX_PATH
#define MAX_PATH 260
#endif

#define WINAPI
#define CALLBACK
#define APIENTRY
#define DECLARE_HANDLE(name) typedef HANDLE name

// Minimal structures
typedef struct tagRECT {
    LONG left;
    LONG top;
    LONG right;
    LONG bottom;
} RECT, *PRECT, *LPRECT;

typedef struct tagPOINT {
    LONG x;
    LONG y;
} POINT, *PPOINT, *LPPOINT;

typedef struct tagSIZE {
    LONG cx;
    LONG cy;
} SIZE, *PSIZE, *LPSIZE;

typedef struct tagMSG {
    HWND hwnd;
    UINT message;
    WPARAM wParam;
    LPARAM lParam;
    DWORD time;
    POINT pt;
} MSG, *PMSG, *LPMSG;

// Stub functions (defined as inline no-ops or simple returns)
inline DWORD GetLastError() { return 0; }
inline void SetLastError(DWORD dwError) { }
inline void OutputDebugStringA(LPCSTR lpOutputString) { }
inline void OutputDebugStringW(LPCWSTR lpOutputString) { }
inline int MessageBoxA(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType) { return 0; }
inline int MessageBoxW(HWND hWnd, LPCWSTR lpText, LPCWSTR lpCaption, UINT uType) { return 0; }

#ifdef UNICODE
#define OutputDebugString OutputDebugStringW
#define MessageBox MessageBoxW
#else
#define OutputDebugString OutputDebugStringA
#define MessageBox MessageBoxA
#endif

// Common defines
#define INFINITE 0xFFFFFFFF
#define INVALID_HANDLE_VALUE ((HANDLE)(LONG_PTR)-1)

// Windows constants
#define MB_OK                       0x00000000L
#define MB_OKCANCEL                 0x00000001L
#define MB_ABORTRETRYIGNORE         0x00000002L
#define MB_YESNOCANCEL              0x00000003L
#define MB_YESNO                    0x00000004L
#define MB_RETRYCANCEL              0x00000005L
#define MB_ICONHAND                 0x00000010L
#define MB_ICONQUESTION             0x00000020L
#define MB_ICONEXCLAMATION          0x00000030L
#define MB_ICONASTERISK             0x00000040L
#define MB_ICONWARNING              MB_ICONEXCLAMATION
#define MB_ICONERROR                MB_ICONHAND
#define MB_ICONINFORMATION          MB_ICONASTERISK
#define MB_ICONSTOP                 MB_ICONHAND

#define IDOK                1
#define IDCANCEL            2
#define IDABORT             3
#define IDRETRY             4
#define IDIGNORE            5
#define IDYES               6
#define IDNO                7

#endif // _MSC_VER

#endif // _CMAKE_STUB_WINDOWS_H_
