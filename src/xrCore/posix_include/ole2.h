// macOS stand-in for the Windows header of the same name.
// Just enough COM for the DirectPlay 8 headers in sdk/include/DPlay to compile.
// No COM objects exist on macOS: CoCreateInstance always fails, so DirectPlay is
// never created and multiplayer reports an error; single player uses the engine's
// direct (in-process) client/server connection and never touches DirectPlay.
#pragma once
#include "../xrCore_platform.h"
#include <wchar.h>

typedef wchar_t WCHAR;
typedef WCHAR* PWSTR;
typedef WCHAR* LPWSTR;
typedef const WCHAR* PCWSTR;
typedef const WCHAR* LPCWSTR;

#ifndef FAR
#define FAR
#endif
#ifndef CONST
#define CONST const
#endif

typedef struct _GUID
{
	DWORD Data1;
	WORD Data2;
	WORD Data3;
	BYTE Data4[8];
} GUID, *LPGUID;

typedef GUID IID;
typedef GUID CLSID;
typedef const GUID& REFGUID;
typedef const IID& REFIID;
typedef const CLSID& REFCLSID;

inline bool operator==(const GUID& a, const GUID& b) { return memcmp(&a, &b, sizeof(GUID)) == 0; }
inline bool operator!=(const GUID& a, const GUID& b) { return !(a == b); }
#define IsEqualGUID(a, b) ((a) == (b))
#define IsEqualIID(a, b) ((a) == (b))

// GUIDs are only compared, never registered, so a per-TU constant is enough
#define DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
	static const GUID name = {l, w1, w2, {b1, b2, b3, b4, b5, b6, b7, b8}}

#define MAKE_HRESULT(sev, fac, code) \
	((HRESULT)(((unsigned long)(sev) << 31) | ((unsigned long)(fac) << 16) | ((unsigned long)(code))))

#define E_NOTIMPL ((HRESULT)0x80004001)
#define E_NOINTERFACE ((HRESULT)0x80004002)
#define E_POINTER ((HRESULT)0x80004003)
#define E_OUTOFMEMORY ((HRESULT)0x8007000E)
#define E_INVALIDARG ((HRESULT)0x80070057)

// interface declaration macros (C++ flavour)
#define interface struct
#define STDMETHODCALLTYPE
#define STDMETHODIMP HRESULT
#define STDMETHODIMP_(type) type
#define STDMETHOD(method) virtual HRESULT method
#define STDMETHOD_(type, method) virtual type method
#define PURE = 0
#define THIS_
#define THIS void
#define DECLARE_INTERFACE(iface) struct iface
#define DECLARE_INTERFACE_(iface, base) struct iface : public base

struct IUnknown
{
	virtual HRESULT QueryInterface(REFIID riid, void** object) = 0;
	virtual ULONG AddRef() = 0;
	virtual ULONG Release() = 0;
	virtual ~IUnknown() = default;
};
typedef IUnknown* LPUNKNOWN;

#define CLSCTX_INPROC_SERVER 0x1
#define CLSCTX_ALL 0x17
#define COINIT_MULTITHREADED 0x0
#define COINIT_APARTMENTTHREADED 0x2

inline HRESULT CoInitialize(void*) { return S_OK; }
inline HRESULT CoInitializeEx(void*, DWORD) { return S_OK; }
inline void CoUninitialize() {}

inline HRESULT CoCreateInstance(REFCLSID, LPUNKNOWN, DWORD, REFIID, void** object)
{
	if (object)
		*object = nullptr;
	return E_NOTIMPL;
}
