# Source list generated from xrNetServer.vcxproj by tools/vcxproj-sources.ps1
add_library(xrNetServer STATIC
	ip_filter.cpp
	NET_AuthCheck.cpp
	NET_Client.cpp
	NET_Common.cpp
	NET_Compressor.cpp
	NET_Log.cpp
	NET_Server.cpp
	stdafx.cpp
)

target_compile_options(xrNetServer PUBLIC "-iquote${CMAKE_CURRENT_SOURCE_DIR}")
target_compile_definitions(xrNetServer PRIVATE XR_NETSERVER_EXPORTS)
target_link_libraries(xrNetServer PUBLIC xrCore)
