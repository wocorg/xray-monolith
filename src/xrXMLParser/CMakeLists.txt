# Source list generated from xrXMLParser.vcxproj by tools/vcxproj-sources.ps1
add_library(xrXMLParser STATIC
	StdAfx.cpp
	tinyxml.cpp
	tinyxmlerror.cpp
	tinyxmlparser.cpp
	xrXMLParser.cpp
)

target_compile_options(xrXMLParser PUBLIC "-iquote${CMAKE_CURRENT_SOURCE_DIR}")
target_compile_definitions(xrXMLParser PRIVATE XRXMLPARSER_EXPORTS)
target_link_libraries(xrXMLParser PUBLIC xrCore xrCDB)
