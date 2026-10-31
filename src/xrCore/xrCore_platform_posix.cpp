// Win32 API subset used by xrCore, implemented on POSIX/macOS.
#include "stdafx.h"

#ifndef _WIN32

#include "xrCore_io_posix.h"
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <utime.h>
#include <pwd.h>
#include <dlfcn.h>
#include <wchar.h>
#include <malloc.h>
#include <errno.h>
#include <map>
#include <mutex>
#include <string>

// from <mach-o/dyld.h>, which can't be included next to the engine's thread_t
extern "C" int _NSGetExecutablePath(char* buf, uint32_t* bufsize);

namespace
{
enum class handle_kind { file, mapping, event };

// every HANDLE this file creates starts with its kind
struct posix_handle
{
	handle_kind kind;
	int fd;
	bool writable;
};

struct posix_event
{
	handle_kind kind;
	pthread_mutex_t mutex;
	pthread_cond_t cond;
	bool manual_reset;
	bool signaled;
};

thread_local DWORD g_last_error = 0;

// Unmapping needs the view size, which Win32 callers don't pass back
std::mutex g_views_lock;
std::map<void*, size_t> g_views;

std::string g_command_line;

void set_error_from_errno() { g_last_error = (DWORD)errno; }
} // namespace

std::string xr_posix_path(const char* path)
{
	std::string s(path ? path : "");
	for (char& c : s)
		if (c == '\\') c = '/';
	return s;
}

void xr_posix_set_command_line(int argc, char** argv)
{
	g_command_line.clear();
	for (int i = 0; i < argc; ++i)
	{
		if (i) g_command_line += ' ';
		g_command_line += argv[i];
	}
}

LPSTR GetCommandLine() { return (LPSTR)g_command_line.c_str(); }

DWORD GetLastError() { return g_last_error; }
void SetLastError(DWORD e) { g_last_error = e; }

// files
HANDLE CreateFile(LPCSTR name, DWORD access, DWORD /*share*/, void* /*security*/, DWORD disposition,
	DWORD /*flags*/, HANDLE /*template_file*/)
{
	bool writable = access & GENERIC_WRITE;
	int flags = writable ? ((access & GENERIC_READ) ? O_RDWR : O_WRONLY) : O_RDONLY;
	switch (disposition)
	{
	case CREATE_ALWAYS: flags |= O_CREAT | O_TRUNC; break;
	case CREATE_NEW: flags |= O_CREAT | O_EXCL; break;
	case OPEN_ALWAYS: flags |= O_CREAT; break;
	case TRUNCATE_EXISTING: flags |= O_TRUNC; break;
	default: break; // OPEN_EXISTING
	}

	int fd = open(xr_posix_path(name).c_str(), flags, 0644);
	if (fd < 0)
	{
		set_error_from_errno();
		return INVALID_HANDLE_VALUE;
	}
	return new posix_handle{handle_kind::file, fd, writable};
}

DWORD GetFileSize(HANDLE h, LPDWORD high)
{
	struct stat st;
	if (h == INVALID_HANDLE_VALUE || fstat(((posix_handle*)h)->fd, &st) != 0)
		return INVALID_FILE_SIZE;
	if (high) *high = DWORD(u64(st.st_size) >> 32);
	return DWORD(st.st_size);
}

BOOL CloseHandle(HANDLE h)
{
	if (!h || h == INVALID_HANDLE_VALUE)
		return FALSE;
	if (*(handle_kind*)h == handle_kind::event)
	{
		auto* e = (posix_event*)h;
		pthread_cond_destroy(&e->cond);
		pthread_mutex_destroy(&e->mutex);
		delete e;
		return TRUE;
	}
	auto* ph = (posix_handle*)h;
	close(ph->fd);
	delete ph;
	return TRUE;
}

// events
HANDLE CreateEvent(LPSECURITY_ATTRIBUTES, BOOL manual_reset, BOOL initial_state, LPCSTR)
{
	auto* e = new posix_event{handle_kind::event, {}, {}, manual_reset != FALSE, initial_state != FALSE};
	pthread_mutex_init(&e->mutex, nullptr);
	pthread_cond_init(&e->cond, nullptr);
	return e;
}

BOOL SetEvent(HANDLE h)
{
	auto* e = (posix_event*)h;
	pthread_mutex_lock(&e->mutex);
	e->signaled = true;
	if (e->manual_reset)
		pthread_cond_broadcast(&e->cond);
	else
		pthread_cond_signal(&e->cond);
	pthread_mutex_unlock(&e->mutex);
	return TRUE;
}

BOOL ResetEvent(HANDLE h)
{
	auto* e = (posix_event*)h;
	pthread_mutex_lock(&e->mutex);
	e->signaled = false;
	pthread_mutex_unlock(&e->mutex);
	return TRUE;
}

DWORD WaitForSingleObject(HANDLE h, DWORD milliseconds)
{
	if (!h || h == INVALID_HANDLE_VALUE || *(handle_kind*)h != handle_kind::event)
		return WAIT_FAILED;

	auto* e = (posix_event*)h;
	pthread_mutex_lock(&e->mutex);
	if (milliseconds == INFINITE)
	{
		while (!e->signaled)
			pthread_cond_wait(&e->cond, &e->mutex);
	}
	else
	{
		timespec deadline;
		clock_gettime(CLOCK_REALTIME, &deadline);
		deadline.tv_sec += milliseconds / 1000;
		deadline.tv_nsec += long(milliseconds % 1000) * 1000000;
		if (deadline.tv_nsec >= 1000000000)
		{
			deadline.tv_sec += 1;
			deadline.tv_nsec -= 1000000000;
		}
		while (!e->signaled)
			if (pthread_cond_timedwait(&e->cond, &e->mutex, &deadline) != 0)
				break;
	}
	DWORD result = e->signaled ? WAIT_OBJECT_0 : WAIT_TIMEOUT;
	if (e->signaled && !e->manual_reset)
		e->signaled = false; // auto-reset: one waiter consumes the signal
	pthread_mutex_unlock(&e->mutex);
	return result;
}

// modules
HMODULE LoadLibrary(LPCSTR name)
{
	return dlopen(xr_posix_path(name).c_str(), RTLD_NOW | RTLD_LOCAL);
}

void* GetProcAddress(HMODULE module, LPCSTR name) { return module ? dlsym(module, name) : nullptr; }

BOOL FreeLibrary(HMODULE module) { return module && dlclose(module) == 0; }

DWORD GetFullPathName(LPCSTR name, DWORD size, LPSTR buffer, LPSTR* file_part)
{
	char resolved[PATH_MAX];
	std::string path = xr_posix_path(name);
	if (!realpath(path.c_str(), resolved))
	{
		// realpath needs an existing file; fall back to joining with the working directory
		char cwd[PATH_MAX];
		if (path.empty() || path[0] == '/' || !getcwd(cwd, sizeof(cwd)))
			strcpy_s(resolved, path.c_str());
		else
			snprintf(resolved, sizeof(resolved), "%s/%s", cwd, path.c_str());
	}
	DWORD len = (DWORD)strlen(resolved);
	if (len + 1 > size)
		return len + 1;
	strcpy_s(buffer, size, resolved);
	if (file_part)
	{
		char* slash = strrchr(buffer, '/');
		*file_part = slash ? slash + 1 : buffer;
	}
	return len;
}

// text conversion: UTF-8 <-> wchar_t (UTF-32 on macOS)
int MultiByteToWideChar(UINT, DWORD, LPCSTR src, int src_len, wchar_t* dst, int dst_len)
{
	if (src_len < 0)
		src_len = (int)strlen(src) + 1;
	int out = 0;
	for (int i = 0; i < src_len;)
	{
		unsigned char c = (unsigned char)src[i];
		u32 cp = c;
		int extra = c >= 0xF0 ? 3 : c >= 0xE0 ? 2 : c >= 0xC0 ? 1 : 0;
		cp = extra == 3 ? (c & 0x07) : extra == 2 ? (c & 0x0F) : extra == 1 ? (c & 0x1F) : c;
		++i;
		for (int k = 0; k < extra && i < src_len; ++k, ++i)
			cp = (cp << 6) | ((unsigned char)src[i] & 0x3F);
		if (dst_len)
		{
			if (out >= dst_len)
				return 0;
			dst[out] = (wchar_t)cp;
		}
		++out;
	}
	return out;
}

int WideCharToMultiByte(UINT, DWORD, const wchar_t* src, int src_len, LPSTR dst, int dst_len, LPCSTR, BOOL* used_default)
{
	if (used_default)
		*used_default = FALSE;
	if (src_len < 0)
		src_len = (int)wcslen(src) + 1;
	int out = 0;
	auto put = [&](char ch) -> bool {
		if (dst_len)
		{
			if (out >= dst_len)
				return false;
			dst[out] = ch;
		}
		++out;
		return true;
	};
	for (int i = 0; i < src_len; ++i)
	{
		u32 cp = (u32)src[i];
		bool ok;
		if (cp < 0x80)
			ok = put((char)cp);
		else if (cp < 0x800)
			ok = put((char)(0xC0 | (cp >> 6))) && put((char)(0x80 | (cp & 0x3F)));
		else if (cp < 0x10000)
			ok = put((char)(0xE0 | (cp >> 12))) && put((char)(0x80 | ((cp >> 6) & 0x3F))) &&
				put((char)(0x80 | (cp & 0x3F)));
		else
			ok = put((char)(0xF0 | (cp >> 18))) && put((char)(0x80 | ((cp >> 12) & 0x3F))) &&
				put((char)(0x80 | ((cp >> 6) & 0x3F))) && put((char)(0x80 | (cp & 0x3F)));
		if (!ok)
			return 0;
	}
	return out;
}

HANDLE CreateFileMapping(HANDLE file, void* /*security*/, DWORD protect, DWORD /*size_high*/, DWORD /*size_low*/,
	LPCSTR /*name*/)
{
	if (file == INVALID_HANDLE_VALUE)
		return nullptr;
	int fd = dup(((posix_handle*)file)->fd);
	if (fd < 0)
	{
		set_error_from_errno();
		return nullptr;
	}
	return new posix_handle{handle_kind::mapping, fd, protect == PAGE_READWRITE};
}

LPVOID MapViewOfFile(HANDLE mapping, DWORD access, DWORD offset_high, DWORD offset_low, SIZE_T size)
{
	if (!mapping || mapping == INVALID_HANDLE_VALUE)
		return nullptr;
	auto* ph = (posix_handle*)mapping;
	off_t offset = off_t((u64(offset_high) << 32) | offset_low);

	if (!size)
	{
		struct stat st;
		if (fstat(ph->fd, &st) != 0)
			return nullptr;
		size = SIZE_T(st.st_size - offset);
	}

	bool write = (access & FILE_MAP_WRITE) && ph->writable;
	void* ptr = mmap(nullptr, size, PROT_READ | (write ? PROT_WRITE : 0), MAP_SHARED, ph->fd, offset);
	if (ptr == MAP_FAILED)
	{
		set_error_from_errno();
		return nullptr;
	}

	std::lock_guard<std::mutex> lock(g_views_lock);
	g_views[ptr] = size;
	return ptr;
}

BOOL UnmapViewOfFile(LPCVOID ptr)
{
	size_t size = 0;
	{
		std::lock_guard<std::mutex> lock(g_views_lock);
		auto it = g_views.find((void*)ptr);
		if (it == g_views.end())
			return FALSE;
		size = it->second;
		g_views.erase(it);
	}
	return munmap((void*)ptr, size) == 0;
}

BOOL ReadFile(HANDLE h, LPVOID buffer, DWORD size, LPDWORD read_bytes, void* /*overlapped*/)
{
	if (h == INVALID_HANDLE_VALUE)
		return FALSE;
	ssize_t n = read(((posix_handle*)h)->fd, buffer, size);
	if (read_bytes) *read_bytes = n > 0 ? DWORD(n) : 0;
	if (n < 0) set_error_from_errno();
	return n >= 0;
}

BOOL WriteFile(HANDLE h, LPCVOID buffer, DWORD size, LPDWORD written, void* /*overlapped*/)
{
	if (h == INVALID_HANDLE_VALUE)
		return FALSE;
	ssize_t n = write(((posix_handle*)h)->fd, buffer, size);
	if (written) *written = n > 0 ? DWORD(n) : 0;
	if (n < 0) set_error_from_errno();
	return n >= 0;
}

DWORD SetFilePointer(HANDLE h, LONG distance, LONG* distance_high, DWORD method)
{
	if (h == INVALID_HANDLE_VALUE)
		return INVALID_SET_FILE_POINTER;
	off_t offset = distance_high ? off_t((u64(u32(*distance_high)) << 32) | u32(distance)) : off_t(distance);
	off_t pos = lseek(((posix_handle*)h)->fd, offset, (int)method);
	if (pos < 0)
	{
		set_error_from_errno();
		return INVALID_SET_FILE_POINTER;
	}
	if (distance_high) *distance_high = LONG(u64(pos) >> 32);
	return DWORD(pos);
}

int MessageBox(HWND /*parent*/, LPCSTR text, LPCSTR caption, UINT /*type*/)
{
	// TODO(macos-port): native dialog once the SDL window layer exists
	fprintf(stderr, "[%s] %s\n", caption ? caption : "", text ? text : "");
	return IDOK;
}

DWORD GetFileAttributes(LPCSTR name)
{
	struct stat st;
	if (stat(xr_posix_path(name).c_str(), &st) != 0)
		return INVALID_FILE_ATTRIBUTES;
	DWORD attr = S_ISDIR(st.st_mode) ? FILE_ATTRIBUTE_DIRECTORY : FILE_ATTRIBUTE_NORMAL;
	if (!(st.st_mode & S_IWUSR))
		attr |= FILE_ATTRIBUTE_READONLY;
	return attr;
}

BOOL SetFileAttributes(LPCSTR name, DWORD attr)
{
	std::string path = xr_posix_path(name);
	struct stat st;
	if (stat(path.c_str(), &st) != 0)
		return FALSE;
	mode_t mode = st.st_mode;
	if (attr & FILE_ATTRIBUTE_READONLY)
		mode &= ~(S_IWUSR | S_IWGRP | S_IWOTH);
	else
		mode |= S_IWUSR;
	return chmod(path.c_str(), mode) == 0;
}

// system
void GetSystemInfo(SYSTEM_INFO* info)
{
	memset(info, 0, sizeof(*info));
	long page = sysconf(_SC_PAGESIZE);
	info->dwPageSize = (DWORD)page;
	// mmap offsets must be page aligned, so the page size is the mapping granularity
	info->dwAllocationGranularity = (DWORD)page;
	info->dwNumberOfProcessors = (DWORD)sysconf(_SC_NPROCESSORS_ONLN);
}

HMODULE GetModuleHandle(LPCSTR /*name*/) { return nullptr; }

DWORD GetModuleFileName(HMODULE /*module*/, LPSTR buffer, DWORD size)
{
	uint32_t len = size;
	if (_NSGetExecutablePath(buffer, &len) != 0)
		return 0;
	char resolved[PATH_MAX];
	if (realpath(buffer, resolved))
		strcpy_s(buffer, size, resolved);
	return (DWORD)strlen(buffer);
}

DWORD GetCurrentDirectory(DWORD size, LPSTR buffer)
{
	if (!getcwd(buffer, size))
		return 0;
	// engine code appends paths to this and expects a trailing separator to be absent
	return (DWORD)strlen(buffer);
}

BOOL SetCurrentDirectory(LPCSTR path) { return chdir(xr_posix_path(path).c_str()) == 0; }

BOOL GetUserName(LPSTR buffer, LPDWORD size)
{
	const char* name = getenv("USER");
	if (!name)
	{
		passwd* pw = getpwuid(getuid());
		name = pw ? pw->pw_name : "player";
	}
	strcpy_s(buffer, *size, name);
	*size = (DWORD)strlen(buffer) + 1;
	return TRUE;
}

BOOL GetComputerName(LPSTR buffer, LPDWORD size)
{
	if (gethostname(buffer, *size) != 0)
		strcpy_s(buffer, *size, "mac");
	buffer[*size - 1] = 0;
	*size = (DWORD)strlen(buffer);
	return TRUE;
}

// CRT
static void copy_part(char* dst, size_t dst_size, const char* begin, const char* end)
{
	if (!dst || !dst_size)
		return;
	size_t n = size_t(end - begin);
	if (n >= dst_size) n = dst_size - 1;
	memcpy(dst, begin, n);
	dst[n] = 0;
}

errno_t _splitpath_s(const char* path, char* drive, size_t drive_size, char* dir, size_t dir_size, char* fname,
	size_t fname_size, char* ext, size_t ext_size)
{
	const char* p = path ? path : "";
	const char* end = p + strlen(p);

	// drive letters only exist on Windows; keep "X:" if a Windows-style path comes in
	const char* start = p;
	if (p[0] && p[1] == ':')
		start = p + 2;
	copy_part(drive, drive_size, p, start);

	const char* last_sep = nullptr;
	for (const char* c = start; c < end; ++c)
		if (*c == '/' || *c == '\\')
			last_sep = c;
	const char* name = last_sep ? last_sep + 1 : start;
	copy_part(dir, dir_size, start, name);

	const char* dot = nullptr;
	for (const char* c = name; c < end; ++c)
		if (*c == '.')
			dot = c;
	if (!dot) dot = end;
	copy_part(fname, fname_size, name, dot);
	copy_part(ext, ext_size, dot, end);
	return 0;
}

void _splitpath(const char* path, char* drive, char* dir, char* fname, char* ext)
{
	_splitpath_s(path, drive, drive ? _MAX_DRIVE : 0, dir, dir ? _MAX_DIR : 0, fname, fname ? _MAX_FNAME : 0, ext,
		ext ? _MAX_EXT : 0);
}

char* _strdate(char* buffer)
{
	time_t t = time(nullptr);
	strftime(buffer, 9, "%m/%d/%y", localtime(&t));
	return buffer;
}

char* _strtime(char* buffer)
{
	time_t t = time(nullptr);
	strftime(buffer, 9, "%H:%M:%S", localtime(&t));
	return buffer;
}

char* _itoa(int value, char* buffer, int radix)
{
	if (radix == 10)
	{
		snprintf(buffer, 33, "%d", value);
		return buffer;
	}
	unsigned v = (unsigned)value;
	char tmp[33];
	int i = 0;
	do
	{
		int d = int(v % (unsigned)radix);
		tmp[i++] = char(d < 10 ? '0' + d : 'a' + d - 10);
		v /= (unsigned)radix;
	}
	while (v);
	int j = 0;
	while (i) buffer[j++] = tmp[--i];
	buffer[j] = 0;
	return buffer;
}

// aligned heap; alignment is stored right before the user block for realloc
void* _aligned_malloc(size_t size, size_t alignment)
{
	if (alignment < sizeof(void*)) alignment = sizeof(void*);
	void* ptr = nullptr;
	if (posix_memalign(&ptr, alignment, size ? size : 1) != 0)
		return nullptr;
	return ptr;
}

void _aligned_free(void* ptr) { free(ptr); }

size_t _aligned_msize(void* ptr, size_t /*alignment*/, size_t /*offset*/) { return ptr ? malloc_size(ptr) : 0; }

void* _aligned_realloc(void* ptr, size_t size, size_t alignment)
{
	if (!ptr)
		return _aligned_malloc(size, alignment);
	if (!size)
	{
		free(ptr);
		return nullptr;
	}
	size_t old_size = malloc_size(ptr);
	void* result = _aligned_malloc(size, alignment);
	if (result)
	{
		memcpy(result, ptr, old_size < size ? old_size : size);
		free(ptr);
	}
	return result;
}

// io.h
int _open(const char* name, int flags, ...)
{
	int mode = 0;
	if (flags & O_CREAT)
	{
		va_list args;
		va_start(args, flags);
		mode = va_arg(args, int);
		va_end(args);
	}
	return open(xr_posix_path(name).c_str(), flags, mode);
}

int _sopen(const char* name, int flags, int /*share*/, ...)
{
	return open(xr_posix_path(name).c_str(), flags, 0644);
}

errno_t _sopen_s(int* handle, const char* name, int flags, int /*share*/, int mode)
{
	*handle = open(xr_posix_path(name).c_str(), flags, mode ? 0644 : 0644);
	return *handle < 0 ? errno : 0;
}

long _filelength(int fd)
{
	struct stat st;
	return fstat(fd, &st) == 0 ? long(st.st_size) : -1;
}

int _mkdir(const char* path) { return mkdir(xr_posix_path(path).c_str(), 0755); }

int _stat(const char* path, struct _stat* st)
{
	struct stat s;
	if (stat(xr_posix_path(path).c_str(), &s) != 0)
		return -1;
	st->st_mode = s.st_mode;
	st->st_size = s.st_size;
	st->st_atime = s.st_atime;
	st->st_mtime = s.st_mtime;
	st->st_ctime = s.st_ctime;
	return 0;
}

int _utime(const char* path, struct _utimbuf* times) { return utime(xr_posix_path(path).c_str(), times); }

// critical sections: recursive mutexes, like Win32
void InitializeCriticalSection(CRITICAL_SECTION* cs)
{
	pthread_mutexattr_t attr;
	pthread_mutexattr_init(&attr);
	pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(cs, &attr);
	pthread_mutexattr_destroy(&attr);
}

void DeleteCriticalSection(CRITICAL_SECTION* cs) { pthread_mutex_destroy(cs); }
void EnterCriticalSection(CRITICAL_SECTION* cs) { pthread_mutex_lock(cs); }
void LeaveCriticalSection(CRITICAL_SECTION* cs) { pthread_mutex_unlock(cs); }
BOOL TryEnterCriticalSection(CRITICAL_SECTION* cs) { return pthread_mutex_trylock(cs) == 0; }

#endif // !_WIN32
