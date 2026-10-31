set(XRCORE_SOURCES
	clsid.cpp
	cpuid.cpp
	crc32.cpp
	dump_string.cpp
	ShaderSourceCRC.cpp
	FileSystem.cpp
	file_stream_reader.cpp
	FS.cpp
	FTimer.cpp
	LocatorAPI.cpp
	LocatorAPI_auth.cpp
	LocatorAPI_defs.cpp
	Lock.cpp
	log.cpp
	LzHuf.cpp
	lzo_compressor.cpp
	memory_allocation_stats.cpp
	memory_monitor.cpp
	memory_usage.cpp
	mezz_stringbuffer.cpp
	Model.cpp
	NET_utils.cpp
	os_clipboard.cpp
	ppmd_compressor.cpp
	rt_compressor.cpp
	rt_compressor9.cpp
	rt_lzo1x_1.cpp
	rt_lzo1x_9x.cpp
	rt_lzo1x_d1.cpp
	rt_lzo1x_d2.cpp
	rt_lzo1x_d3.cpp
	rt_lzo_init.cpp
	ScopeLock.cpp
	stream_reader.cpp
	string_concatenations.cpp
	xrCore.cpp
	xrDebug_posix.cpp
	xrMemory.cpp
	xrMemory_align.cpp
	xrMemory_debug.cpp
	xrMemory_POOL.cpp
	xrMemory_pso_Copy.cpp
	xrMemory_pso_Fill.cpp
	xrMemory_pso_Fill32.cpp
	xrMemory_subst_msvc.cpp
	xrsharedmem.cpp
	xrstring.cpp
	xrSyncronize.cpp
	Xr_ini.cpp
	xr_shared.cpp
	xr_trims.cpp
	_compressed_normal.cpp
	_math.cpp
	_sphere.cpp
	_std_extensions.cpp
	xrCore_platform_posix.cpp
)
# Not built on macOS: xrDebugNew.cpp (replaced by xrDebug_posix.cpp),
# blackbox/* (Win32 crash handler), FileSystem_borland.cpp,
# xrMemory_subst_borland.cpp, xrDebug.cpp and LocatorAPI_Notifications.cpp
# (both already excluded in xrCore.vcxproj).

add_library(xrCore STATIC ${XRCORE_SOURCES})

target_include_directories(xrCore PUBLIC
	${XRAY_SRC_DIR}
	"${XRAY_SRC_DIR}/3rd party"
	${XRAY_SDK_DIR}/include
)
# Stand-ins for Windows-only headers (io.h, direct.h, malloc.h...)
target_include_directories(xrCore SYSTEM PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/posix_include)

# xrCore has headers named like system ones (_types.h, log.h, vector.h...),
# so expose its directory to "" includes only, never to <> includes.
target_compile_options(xrCore PUBLIC "-iquote${CMAKE_CURRENT_SOURCE_DIR}")

target_compile_definitions(xrCore PRIVATE XRCORE_EXPORTS)

find_package(TBB REQUIRED)
target_link_libraries(xrCore PUBLIC TBB::tbb sse2neon)
