add_library(luabind STATIC
	luabind/src/class.cpp
	luabind/src/class_info.cpp
	luabind/src/class_registry.cpp
	luabind/src/class_rep.cpp
	luabind/src/create_class.cpp
	luabind/src/error.cpp
	luabind/src/find_best_match.cpp
	luabind/src/function.cpp
	luabind/src/implicit_cast.cpp
	luabind/src/link_compatibility.cpp
	luabind_memory.cpp
	luabind/src/object.cpp
	luabind/src/object_rep.cpp
	luabind/src/open.cpp
	luabind/src/pcall.cpp
	luabind/src/ref.cpp
	luabind/src/scope.cpp
	luabind/src/stack_content_by_name.cpp
	luabind/src/weak_ref.cpp
	luabind/src/wrapper_base.cpp
)

# headers are included as <luabind/...>, relative to "src/3rd party"
target_include_directories(luabind PUBLIC "${XRAY_SRC_DIR}/3rd party")
target_include_directories(luabind PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/luabind/src ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(luabind PUBLIC LUABIND_NO_EXCEPTIONS PRIVATE LUABIND_BUILDING)
target_link_libraries(luabind PUBLIC PkgConfig::LUAJIT xrCore)
