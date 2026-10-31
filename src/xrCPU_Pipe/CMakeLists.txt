# Source list generated from xrCPU_Pipe.vcxproj by tools/vcxproj-sources.ps1
add_library(xrCPU_Pipe STATIC
	PLC.cpp
	StdAfx.cpp
	ttapi.cpp
	xrCPU_Pipe.cpp
	xrSkin2W.cpp
	xrSkin2W_SSE.cpp
	xrSkin2W_thread.cpp
)

target_compile_options(xrCPU_Pipe PUBLIC "-iquote${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(xrCPU_Pipe PUBLIC xrCore)
