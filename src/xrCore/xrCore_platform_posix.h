// Win32 compatibility layer for non-Windows builds (macOS port).
// Provides the Windows types, macros and CRT functions the engine relies on.
#ifndef XRCORE_PLATFORM_POSIX_H
#define XRCORE_PLATFORM_POSIX_H
#pragma once

// Code tells MSVC-only paths apart with _MSC_VER; clang's MS-compatibility mode must
// not define it here (see -fms-compatibility-version=0 in CMakeLists.txt)
#ifdef _MSC_VER
#error "_MSC_VER is defined in a non-Windows build: Windows-only code paths would be compiled"
#endif

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
typedef void VOID;
typedef size_t SIZE_T;
typedef intptr_t INT_PTR;
typedef uintptr_t UINT_PTR;
typedef intptr_t LONG_PTR;
typedef uintptr_t ULONG_PTR;
typedef uintptr_t DWORD_PTR;

typedef unsigned short USHORT;
typedef USHORT* PUSHORT;
typedef DWORD* PDWORD;
typedef WORD* PWORD;
typedef WORD* LPWORD;
typedef BYTE* PBYTE;
typedef LONG* PLONG;

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
typedef void* HMONITOR;
typedef void* HDC;
typedef void* HICON;
typedef void* HCURSOR;
typedef uint32_t UINT32;
typedef int32_t INT32;
typedef uint64_t UINT64;
typedef int64_t INT64;
typedef UINT_PTR WPARAM;
typedef LONG_PTR LPARAM;
typedef LONG_PTR LRESULT;
typedef LONG HRESULT;

#define RGB(r, g, b) ((DWORD)(((BYTE)(r) | ((WORD)((BYTE)(g)) << 8)) | (((DWORD)(BYTE)(b)) << 16)))
#define GetRValue(rgb) ((BYTE)(rgb))
#define GetGValue(rgb) ((BYTE)(((WORD)(rgb)) >> 8))
#define GetBValue(rgb) ((BYTE)((rgb) >> 16))

#define S_OK ((HRESULT)0)
#define S_FALSE ((HRESULT)1)
#define E_FAIL ((HRESULT)0x80004005)
#define SUCCEEDED(hr) (((HRESULT)(hr)) >= 0)
#define FAILED(hr) (((HRESULT)(hr)) < 0)

typedef struct tagRECT
{
	LONG left, top, right, bottom;
} RECT, *LPRECT;

typedef struct tagPOINT
{
	LONG x, y;
} POINT, *LPPOINT;

#define DLL_PROCESS_DETACH 0
#define DLL_PROCESS_ATTACH 1
#define DLL_THREAD_ATTACH 2
#define DLL_THREAD_DETACH 3

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
#define strcmpi strcasecmp
#define _strcmpi strcasecmp
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
#ifndef _ERRNO_T
typedef int errno_t;
#endif

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

template <size_t N>
inline int vsprintf_s(char (&dst)[N], const char* fmt, va_list args) { return vsnprintf(dst, N, fmt, args); }

#define _atoi64(s) strtoll((s), nullptr, 10)
#define _strtoui64 strtoull
#define _strtoi64 strtoll

inline errno_t _i64toa_s(long long value, char* buffer, size_t size, int radix)
{
	if (radix == 16) snprintf(buffer, size, "%llx", value);
	else snprintf(buffer, size, "%lld", value);
	return 0;
}

inline errno_t _ui64toa_s(unsigned long long value, char* buffer, size_t size, int radix)
{
	if (radix == 16) snprintf(buffer, size, "%llx", value);
	else snprintf(buffer, size, "%llu", value);
	return 0;
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

// threads
inline BOOL SwitchToThread()
{
	sched_yield();
	return TRUE;
}

inline DWORD GetCurrentThreadId()
{
	uint64_t id = 0;
	pthread_threadid_np(nullptr, &id);
	return (DWORD)id;
}

// timing
inline void Sleep(DWORD ms) { usleep((useconds_t)ms * 1000); }

typedef struct _SYSTEMTIME
{
	WORD wYear;
	WORD wMonth;
	WORD wDayOfWeek;
	WORD wDay;
	WORD wHour;
	WORD wMinute;
	WORD wSecond;
	WORD wMilliseconds;
} SYSTEMTIME, *LPSYSTEMTIME;

inline void xr_fill_systemtime(SYSTEMTIME* st, bool local)
{
	timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	time_t secs = ts.tv_sec;
	tm t;
	if (local) localtime_r(&secs, &t);
	else gmtime_r(&secs, &t);
	st->wYear = WORD(t.tm_year + 1900);
	st->wMonth = WORD(t.tm_mon + 1);
	st->wDayOfWeek = WORD(t.tm_wday);
	st->wDay = WORD(t.tm_mday);
	st->wHour = WORD(t.tm_hour);
	st->wMinute = WORD(t.tm_min);
	st->wSecond = WORD(t.tm_sec);
	st->wMilliseconds = WORD(ts.tv_nsec / 1000000);
}

inline void GetLocalTime(LPSYSTEMTIME st) { xr_fill_systemtime(st, true); }
inline void GetSystemTime(LPSYSTEMTIME st) { xr_fill_systemtime(st, false); }

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

typedef char* PSTR;
typedef LARGE_INTEGER* PLARGE_INTEGER;

#define CopyMemory(dst, src, size) memcpy((dst), (src), (size))
#define MoveMemory(dst, src, size) memmove((dst), (src), (size))
#define FillMemory(dst, size, value) memset((dst), (value), (size))
#define ZeroMemory(dst, size) memset((dst), 0, (size))

// --- implemented in xrCore_platform_posix.cpp ---
#include <string>
std::string xr_posix_path(const char* path); // backslashes -> slashes
void xr_posix_set_command_line(int argc, char** argv);

LPSTR GetCommandLine();
DWORD GetLastError();
void SetLastError(DWORD e);

#define INVALID_HANDLE_VALUE ((HANDLE)(intptr_t)-1)
#define INVALID_FILE_SIZE ((DWORD)0xFFFFFFFF)
#define INVALID_FILE_ATTRIBUTES ((DWORD)-1)
#define GENERIC_READ 0x80000000u
#define GENERIC_WRITE 0x40000000u
#define FILE_SHARE_READ 0x1
#define FILE_SHARE_WRITE 0x2
#define FILE_SHARE_DELETE 0x4
#define CREATE_NEW 1
#define CREATE_ALWAYS 2
#define OPEN_EXISTING 3
#define OPEN_ALWAYS 4
#define TRUNCATE_EXISTING 5
#define FILE_ATTRIBUTE_READONLY 0x1
#define FILE_ATTRIBUTE_HIDDEN 0x2
#define FILE_ATTRIBUTE_DIRECTORY 0x10
#define FILE_ATTRIBUTE_NORMAL 0x80
#define FILE_FLAG_NO_BUFFERING 0x20000000
#define FILE_FLAG_SEQUENTIAL_SCAN 0x08000000
#define PAGE_READONLY 0x02
#define PAGE_READWRITE 0x04
#define FILE_MAP_WRITE 0x2
#define FILE_MAP_READ 0x4
#define FILE_MAP_ALL_ACCESS 0xF001F

HANDLE CreateFile(LPCSTR name, DWORD access, DWORD share, void* security, DWORD disposition, DWORD flags,
	HANDLE template_file);
DWORD GetFileSize(HANDLE h, LPDWORD high);
BOOL CloseHandle(HANDLE h);
HANDLE CreateFileMapping(HANDLE file, void* security, DWORD protect, DWORD size_high, DWORD size_low, LPCSTR name);
LPVOID MapViewOfFile(HANDLE mapping, DWORD access, DWORD offset_high, DWORD offset_low, SIZE_T size);
BOOL UnmapViewOfFile(LPCVOID ptr);
DWORD GetFileAttributes(LPCSTR name);
BOOL SetFileAttributes(LPCSTR name, DWORD attr);

#define FILE_BEGIN SEEK_SET
#define FILE_CURRENT SEEK_CUR
#define FILE_END SEEK_END
#define INVALID_SET_FILE_POINTER ((DWORD)-1)
BOOL ReadFile(HANDLE h, LPVOID buffer, DWORD size, LPDWORD read, void* overlapped);
BOOL WriteFile(HANDLE h, LPCVOID buffer, DWORD size, LPDWORD written, void* overlapped);
DWORD SetFilePointer(HANDLE h, LONG distance, LONG* distance_high, DWORD method);

// Only used as a debug sanity check before reading memory
inline BOOL IsBadReadPtr(const void* ptr, UINT_PTR) { return ptr == nullptr; }

#define MB_OK 0x0
#define MB_OKCANCEL 0x1
#define MB_YESNO 0x4
#define MB_ICONERROR 0x10
#define MB_ICONWARNING 0x30
#define MB_ICONINFORMATION 0x40
#define MB_SYSTEMMODAL 0x1000
#define MB_TOPMOST 0x40000
#define IDOK 1
#define IDCANCEL 2
#define IDYES 6
#define IDNO 7
int MessageBox(HWND parent, LPCSTR text, LPCSTR caption, UINT type);

struct SYSTEM_INFO
{
	DWORD dwPageSize;
	DWORD dwAllocationGranularity;
	DWORD dwNumberOfProcessors;
};
void GetSystemInfo(SYSTEM_INFO* info);

HMODULE GetModuleHandle(LPCSTR name);
DWORD GetModuleFileName(HMODULE module, LPSTR buffer, DWORD size);
DWORD GetCurrentDirectory(DWORD size, LPSTR buffer);
BOOL SetCurrentDirectory(LPCSTR path);
BOOL GetUserName(LPSTR buffer, LPDWORD size);
BOOL GetComputerName(LPSTR buffer, LPDWORD size);

errno_t _splitpath_s(const char* path, char* drive, size_t drive_size, char* dir, size_t dir_size, char* fname,
	size_t fname_size, char* ext, size_t ext_size);
void _splitpath(const char* path, char* drive, char* dir, char* fname, char* ext);
char* _strdate(char* buffer);
char* _strtime(char* buffer);
#define _tzset tzset
char* _itoa(int value, char* buffer, int radix);
#define itoa _itoa
#define _getcwd getcwd
#define _rmdir rmdir

void* _aligned_malloc(size_t size, size_t alignment);
void* _aligned_realloc(void* ptr, size_t size, size_t alignment);
void _aligned_free(void* ptr);
size_t _aligned_msize(void* ptr, size_t alignment, size_t offset);

typedef pthread_mutex_t CRITICAL_SECTION;
void InitializeCriticalSection(CRITICAL_SECTION* cs);
void DeleteCriticalSection(CRITICAL_SECTION* cs);
void EnterCriticalSection(CRITICAL_SECTION* cs);
void LeaveCriticalSection(CRITICAL_SECTION* cs);
BOOL TryEnterCriticalSection(CRITICAL_SECTION* cs);

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

#define _InterlockedIncrement InterlockedIncrement
#define _InterlockedDecrement InterlockedDecrement
#define _InterlockedExchange InterlockedExchange
#define _InterlockedCompareExchange InterlockedCompareExchange

#define INFINITE 0xFFFFFFFF
#define sscanf_s sscanf

#endif // XRCORE_PLATFORM_POSIX_H
