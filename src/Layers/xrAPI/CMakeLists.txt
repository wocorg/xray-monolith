# Source list generated from xrAPI.vcxproj by tools/vcxproj-sources.ps1
add_library(xrAPI STATIC
	stdafx.cpp
	xrAPI.cpp
)

target_compile_options(xrAPI PUBLIC "-iquote${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(xrAPI PUBLIC xrCore)
