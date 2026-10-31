// _findfirst/_findnext/_findclose (MSVC <io.h>) on top of opendir/fnmatch.
#ifndef XRCORE_IO_POSIX_H
#define XRCORE_IO_POSIX_H
#pragma once

#include <dirent.h>
#include <fnmatch.h>
#include <sys/stat.h>
#include <string>

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
