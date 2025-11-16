/*
**	CMake oaidl.h Stub Header
**	Provides minimal OLE Automation definitions for non-Windows builds
*/

#ifndef _CMAKE_STUB_OAIDL_H_
#define _CMAKE_STUB_OAIDL_H_

#ifndef _MSC_VER

// Minimal COM/OLE stubs
typedef void* IUnknown;
typedef void* IDispatch;
typedef void* VARIANT;
typedef long HRESULT;
typedef unsigned short VARTYPE;

#define S_OK 0
#define E_FAIL 0x80004005L
#define SUCCEEDED(hr) ((HRESULT)(hr) >= 0)
#define FAILED(hr) ((HRESULT)(hr) < 0)

#endif // _MSC_VER

#endif // _CMAKE_STUB_OAIDL_H_
