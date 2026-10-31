// _findfirst/_findnext/_findclose (MSVC <io.h>) on top of opendir/fnmatch.
#ifndef XRCORE_IO_POSIX_H
#define XRCORE_IO_POSIX_H
#pragma once

#include <dirent.h>
#include <fnmatch.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <utime.h>
#include <cstring>
#include <string>

// open flags; POSIX has no text/binary distinction
#define O_BINARY 0
#define O_SEQUENTIAL 0
#define _O_BINARY 0
#define _O_RDONLY O_RDONLY
#define _O_WRONLY O_WRONLY
#define _O_RDWR O_RDWR
#define _O_CREAT O_CREAT
#define _O_TRUNC O_TRUNC
#define _O_APPEND O_APPEND
#define _SH_DENYNO 0x40
#define _SH_DENYWR 0x20
#define SH_DENYNO _SH_DENYNO
#define SH_DENYWR _SH_DENYWR
#define _S_IREAD S_IRUSR
#define _S_IWRITE S_IWUSR

int _open(const char* name, int flags, ...);
int _sopen(const char* name, int flags, int share, ...);
errno_t _sopen_s(int* handle, const char* name, int flags, int share, int mode);
long _filelength(int fd);
#define filelength _filelength
#define _read read
#define _write write
#define _close close
#define _lseek lseek
#define _tell(fd) lseek((fd), 0, SEEK_CUR)
#define _fdopen fdopen
int _mkdir(const char* path);

// macOS defines st_atime/st_mtime/st_ctime as macros over these timespec fields
struct _stat
{
	mode_t st_mode;
	off_t st_size;
	struct timespec st_atimespec;
	struct timespec st_mtimespec;
	struct timespec st_ctimespec;
};
int _stat(const char* path, struct _stat* st);
int _utime(const char* path, struct utimbuf* times);

// MSVC exposes strerror texts as an array
struct xr_sys_errlist_t
{
	const char* operator[](int e) const { return strerror(e); }
};
static const xr_sys_errlist_t _sys_errlist = {};

#define _A_NORMAL 0x00
#define _A_RDONLY 0x01
#define _A_HIDDEN 0x02
#define _A_SYSTEM 0x04
#define _A_SUBDIR 0x10
#define _A_ARCH 0x20

typedef unsigned long _fsize_t;

struct _finddata_t
{
	unsigned attrib;
	time_t time_create;
	time_t time_access;
	time_t time_write;
	_fsize_t size;
	char name[260];
};
#define _finddata64i32_t _finddata_t
#define _findfirst64i32 _findfirst
#define _findnext64i32 _findnext

namespace xr_io_posix
{
struct find_handle
{
	DIR* dir;
	std::string folder;
	std::string mask;
};

inline std::string to_posix_path(const char* path)
{
	std::string s(path);
	for (char& c : s)
		if (c == '\\') c = '/';
	return s;
}

inline bool next_match(find_handle* h, _finddata_t* data)
{
	while (dirent* e = readdir(h->dir))
	{
		if (fnmatch(h->mask.c_str(), e->d_name, FNM_CASEFOLD) != 0)
			continue;

		std::string full = h->folder + e->d_name;
		struct stat st;
		if (stat(full.c_str(), &st) != 0)
			continue;

		data->attrib = S_ISDIR(st.st_mode) ? _A_SUBDIR : _A_NORMAL;
		if (e->d_name[0] == '.' && e->d_name[1] && !(e->d_name[1] == '.' && !e->d_name[2]))
			data->attrib |= _A_HIDDEN;
		if (!(st.st_mode & S_IWUSR))
			data->attrib |= _A_RDONLY;
		data->time_create = st.st_birthtimespec.tv_sec;
		data->time_access = st.st_atimespec.tv_sec;
		data->time_write = st.st_mtimespec.tv_sec;
		data->size = (_fsize_t)st.st_size;
		strncpy(data->name, e->d_name, sizeof(data->name) - 1);
		data->name[sizeof(data->name) - 1] = 0;
		return true;
	}
	return false;
}
} // namespace xr_io_posix

inline intptr_t _findfirst(const char* filespec, _finddata_t* data)
{
	std::string spec = xr_io_posix::to_posix_path(filespec);
	size_t slash = spec.rfind('/');
	std::string folder = slash == std::string::npos ? std::string("./") : spec.substr(0, slash + 1);
	std::string mask = slash == std::string::npos ? spec : spec.substr(slash + 1);
	if (mask == "*.*")
		mask = "*";

	DIR* dir = opendir(folder.c_str());
	if (!dir)
		return -1;

	auto* h = new xr_io_posix::find_handle{dir, folder, mask};
	if (!xr_io_posix::next_match(h, data))
	{
		closedir(dir);
		delete h;
		return -1;
	}
	return (intptr_t)h;
}

inline int _findnext(intptr_t handle, _finddata_t* data)
{
	return xr_io_posix::next_match((xr_io_posix::find_handle*)handle, data) ? 0 : -1;
}

inline int _findclose(intptr_t handle)
{
	auto* h = (xr_io_posix::find_handle*)handle;
	closedir(h->dir);
	delete h;
	return 0;
}

#endif // XRCORE_IO_POSIX_H
