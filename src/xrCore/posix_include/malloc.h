// macOS stand-in for the MSVC/Windows header of the same name
#pragma once
#include <stdlib.h>
// declared here instead of <malloc/malloc.h>: that pulls in mach's thread_t,
// which clashes with the engine's thread_t
extern "C" size_t malloc_size(const void* ptr);
#define _msize(p) malloc_size(p)
#define _expand(p, size) ((void*)0)
