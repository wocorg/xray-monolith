// macOS stand-in for the MSVC/Windows header of the same name
#pragma once
#if defined(__aarch64__)
// x86 SSE intrinsics translated to ARM NEON
#include <sse2neon.h>
#else
#include_next <pmmintrin.h>
#endif
