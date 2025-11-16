/*
**	CMake tchar.h Stub Header
**	Provides TCHAR definitions for non-Windows builds
*/

#ifndef _CMAKE_STUB_TCHAR_H_
#define _CMAKE_STUB_TCHAR_H_

#include <string.h>
#include <wchar.h>
#include <ctype.h>

#ifdef UNICODE
    typedef wchar_t TCHAR;
    #define _T(x) L##x
    #define _TEXT(x) L##x
    #define _tmain wmain
    #define _tcslen wcslen
    #define _tcscpy wcscpy
    #define _tcscat wcscat
    #define _tcscmp wcscmp
    #define _tcsicmp wcscasecmp
    #define _tcsnicmp wcsncasecmp
    #define _tcschr wcschr
    #define _tcsrchr wcsrchr
    #define _tcsstr wcsstr
    #define _tcstok wcstok
    #define _tprintf wprintf
    #define _ftprintf fwprintf
    #define _stprintf swprintf
    #define _sntprintf snwprintf
    #define _tscanf wscanf
    #define _ftscanf fwscanf
    #define _stscanf swscanf
#else
    typedef char TCHAR;
    #define _T(x) x
    #define _TEXT(x) x
    #define _tmain main
    #define _tcslen strlen
    #define _tcscpy strcpy
    #define _tcscat strcat
    #define _tcscmp strcmp
    #define _tcsicmp strcasecmp
    #define _tcsnicmp strncasecmp
    #define _tcschr strchr
    #define _tcsrchr strrchr
    #define _tcsstr strstr
    #define _tcstok strtok
    #define _tprintf printf
    #define _ftprintf fprintf
    #define _stprintf sprintf
    #define _sntprintf snprintf
    #define _tscanf scanf
    #define _ftscanf fscanf
    #define _stscanf sscanf
#endif

#endif // _CMAKE_STUB_TCHAR_H_
