# CMake build for the macOS port. The Windows build still uses src/engine-vs2022.sln.
cmake_minimum_required(VERSION 3.21)
project(xray-monolith LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Release)
endif()

set(XRAY_SRC_DIR ${CMAKE_SOURCE_DIR}/src)
set(XRAY_SDK_DIR ${CMAKE_SOURCE_DIR}/sdk)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
	# MSVC-isms used throughout the engine: __declspec, anonymous structs, __int64
	add_compile_options(
		-fms-extensions
		-fdeclspec
		# MSVC resolves names in templates at instantiation; the engine relies on it
		-fdelayed-template-parsing
		# the engine has methods named or/and/xor (e.g. _flags::or)
		-fno-operator-names
		-ferror-limit=0
		-Wno-unknown-pragmas
		-Wno-microsoft
		-Wno-ignored-attributes
		-Wno-c++11-narrowing
		# old C code (lzo) uses the 'register' keyword
		-Wno-register
	)
endif()

add_compile_definitions(NDEBUG PURE_ALLOC)

# sse2neon: the engine uses x86 SSE intrinsics directly; on arm64 the stand-in
# <xmmintrin.h> etc. in src/xrCore/posix_include map them to NEON
include(FetchContent)
FetchContent_Declare(sse2neon
	GIT_REPOSITORY https://github.com/DLTcollab/sse2neon.git
	GIT_TAG v1.7.0
	GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(sse2neon)
add_library(sse2neon INTERFACE)
target_include_directories(sse2neon SYSTEM INTERFACE ${sse2neon_SOURCE_DIR})

# LuaJIT 2.1 (Homebrew): 2.0.4 vendored in src/3rd party/luajit-2 has no arm64 support
find_package(PkgConfig REQUIRED)
pkg_check_modules(LUAJIT REQUIRED IMPORTED_TARGET luajit)

# sound: Ogg Vorbis and OpenAL Soft (Homebrew; Windows builds use the vendored OpenAL-new)
pkg_check_modules(VORBIS REQUIRED IMPORTED_TARGET vorbisfile vorbis ogg)
pkg_check_modules(OPENAL REQUIRED IMPORTED_TARGET openal)

add_subdirectory(src/xrCore)
add_subdirectory("src/3rd party/luabind")
add_subdirectory("src/3rd party/ode")
add_subdirectory(src/xrCDB)
add_subdirectory(src/Layers/xrAPI)
add_subdirectory(src/xrCPU_Pipe)
add_subdirectory(src/xrSound)
add_subdirectory(src/xrParticles)
add_subdirectory(src/xrPhysics)
# TODO(macos-port): xrNetServer is built on DirectPlay 8 (Windows-only); needs its own transport
#add_subdirectory(src/xrNetServer)
add_subdirectory(src/xrXMLParser)
