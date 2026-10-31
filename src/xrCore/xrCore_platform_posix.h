// Win32 compatibility layer for non-Windows builds (macOS port).
// Provides the Windows types, macros and CRT functions the engine relies on.
#ifndef XRCORE_PLATFORM_POSIX_H
#define XRCORE_PLATFORM_POSIX_H
#pragma once

#include <cstdint>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdarg>
#include <cctype>
#include <cmath>
#include <unordered_map>
#include <climits>
#include <ctime>
#include <strings.h>
#include <unistd.h>
#include <alloca.h>
#include <pthread.h>

// types
typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef uint32_t DWORD;
typedef int BOOL;
typedef int INT;
typedef unsigned int UINT;
typedef int32_t LONG;
typedef uint32_t ULONG;
typedef int64_t LONGLONG;
typedef uint64_t ULONGLONG;
typedef char CHAR;
typedef float FLOAT;
typedef size_t SIZE_T;
typedef intptr_t INT_PTR;
typedef uintptr_t UINT_PTR;
typedef intptr_t LONG_PTR;
typedef uintptr_t ULONG_PTR;
typedef uintptr_t DWORD_PTR;

typedef void* PVOID;
typedef void* LPVOID;
typedef const void* LPCVOID;
typedef char* LPSTR;
typedef const char* LPCSTR;
typedef BYTE* LPBYTE;
typedef DWORD* LPDWORD;
typedef LONG* LPLONG;

typedef void* HANDLE;
typedef void* HMODULE;
typedef void* HINSTANCE;
typedef void* HWND;

typedef union _LARGE_INTEGER
{
	struct
	{
		DWORD LowPart;
		LONG HighPart;
	};
	LONGLONG QuadPart;
} LARGE_INTEGER;

#ifndef TRUE
# define TRUE 1
#endif
#ifndef FALSE
# define FALSE 0
#endif
#ifndef MAX_PATH
# define MAX_PATH 260
#endif
#define _MAX_PATH 260
#define _MAX_DRIVE 3
#define _MAX_DIR 256
#define _MAX_FNAME 256
#define _MAX_EXT 256

#define WINAPI
#define CALLBACK
#define APIENTRY
#ifndef __stdcall
# define __stdcall
#endif
#ifndef __cdecl
# define __cdecl
#endif

// CRT name differences
#define _stricmp strcasecmp
#define stricmp strcasecmp
#define _strnicmp strncasecmp
#define strnicmp strncasecmp
#define _snprintf snprintf
#define _vsnprintf vsnprintf
#define _alloca alloca
#define _copysign copysign
#define _access access
#define _unlink unlink

inline char* _strlwr(char* s)
{
	for (char* p = s; *p; ++p) *p = (char)tolower((unsigned char)*p);
	return s;
}

inline char* _strupr(char* s)
{
	for (char* p = s; *p; ++p) *p = (char)toupper((unsigned char)*p);
	return s;
}

#define strlwr _strlwr
#define strupr _strupr

// secure CRT functions; return 0 on success like MSVC
typedef int errno_t;

inline int strncpy_s(char* dst, size_t size, const char* src, size_t count)
{
	if (!dst || !size) return 22;
	size_t n = src ? strnlen(src, count) : 0;
	if (n >= size) n = size - 1;
	memcpy(dst, src, n);
	dst[n] = 0;
	return 0;
}

inline int vsnprintf_s(char* dst, size_t size, size_t count, const char* fmt, va_list args)
{
	size_t limit = count + 1 < size ? count + 1 : size;
	return vsnprintf(dst, limit, fmt, args);
}

inline int strcpy_s(char* dst, size_t size, const char* src)
{
	if (!dst || !size) return 22;
	strncpy(dst, src ? src : "", size - 1);
	dst[size - 1] = 0;
	return 0;
}

template <size_t N>
inline int strcpy_s(char (&dst)[N], const char* src) { return strcpy_s(dst, N, src); }

inline int strcat_s(char* dst, size_t size, const char* src)
{
	if (!dst || !size) return 22;
	size_t len = strnlen(dst, size);
	if (len >= size) return 22;
	return strcpy_s(dst + len, size - len, src);
}

template <size_t N>
inline int strcat_s(char (&dst)[N], const char* src) { return strcat_s(dst, N, src); }

inline int vsprintf_s(char* dst, size_t size, const char* fmt, va_list args)
{
	return vsnprintf(dst, size, fmt, args);
}

inline int sprintf_s(char* dst, size_t size, const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	int r = vsnprintf(dst, size, fmt, args);
	va_end(args);
	return r;
}

template <size_t N>
inline int sprintf_s(char (&dst)[N], const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	int r = vsnprintf(dst, N, fmt, args);
	va_end(args);
	return r;
}

// timing
inline void Sleep(DWORD ms) { usleep((useconds_t)ms * 1000); }

inline DWORD GetTickCount()
{
	timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (DWORD)(ts.tv_sec * 1000ull + ts.tv_nsec / 1000000ull);
}

inline BOOL QueryPerformanceFrequency(LARGE_INTEGER* f)
{
	f->QuadPart = 1000000000ll;
	return TRUE;
}

inline BOOL QueryPerformanceCounter(LARGE_INTEGER* c)
{
	c->QuadPart = (LONGLONG)clock_gettime_nsec_np(CLOCK_UPTIME_RAW);
	return TRUE;
}

// slim reader/writer lock
typedef pthread_rwlock_t SRWLOCK;
typedef SRWLOCK* PSRWLOCK;
inline void InitializeSRWLock(PSRWLOCK l) { pthread_rwlock_init(l, nullptr); }
inline void AcquireSRWLockExclusive(PSRWLOCK l) { pthread_rwlock_wrlock(l); }
inline void ReleaseSRWLockExclusive(PSRWLOCK l) { pthread_rwlock_unlock(l); }
inline void AcquireSRWLockShared(PSRWLOCK l) { pthread_rwlock_rdlock(l); }
inline void ReleaseSRWLockShared(PSRWLOCK l) { pthread_rwlock_unlock(l); }
inline BOOL TryAcquireSRWLockExclusive(PSRWLOCK l) { return pthread_rwlock_trywrlock(l) == 0; }
inline BOOL TryAcquireSRWLockShared(PSRWLOCK l) { return pthread_rwlock_tryrdlock(l) == 0; }

// atomics
inline LONG InterlockedIncrement(volatile LONG* v) { return __atomic_add_fetch(v, 1, __ATOMIC_SEQ_CST); }
inline LONG InterlockedDecrement(volatile LONG* v) { return __atomic_sub_fetch(v, 1, __ATOMIC_SEQ_CST); }
inline LONG InterlockedExchange(volatile LONG* v, LONG x) { return __atomic_exchange_n(v, x, __ATOMIC_SEQ_CST); }

inline LONG InterlockedCompareExchange(volatile LONG* v, LONG x, LONG cmp)
{
	__atomic_compare_exchange_n(v, &cmp, x, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
	return cmp;
}

#endif // XRCORE_PLATFORM_POSIX_H
