# Source list generated from xrParticles.vcxproj by tools/vcxproj-sources.ps1
add_library(xrParticles STATIC
	noise.cpp
	particle_actions.cpp
	particle_actions_collection.cpp
	particle_actions_collection_io.cpp
	particle_core.cpp
	particle_effect.cpp
	particle_manager.cpp
	stdafx.cpp
)

target_compile_options(xrParticles PUBLIC "-iquote${CMAKE_CURRENT_SOURCE_DIR}")
target_compile_definitions(xrParticles PRIVATE XR_PARTICLES_EXPORTS)
target_link_libraries(xrParticles PUBLIC xrCore xrCPU_Pipe)
