// macOS stand-in for the MSVC/Windows header of the same name
#pragma once
#if defined(__aarch64__)
// x86 SSE intrinsics translated to ARM NEON
#include <sse2neon.h>
// sse2neon defines __int64 as int64_t, which breaks "signed __int64";
// clang already has the __int64 keyword in MS-extensions mode
#undef __int64
#else
#include_next <emmintrin.h>
#endif
