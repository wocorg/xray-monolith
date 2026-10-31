# Source list generated from xrSound.vcxproj by tools/vcxproj-sources.ps1
add_library(xrSound STATIC
	guids.cpp
	NotificationClient.cpp
	OpenALDeviceList.cpp
	sound.cpp
	SoundRender_Cache.cpp
	SoundRender_Core.cpp
	SoundRender_CoreA.cpp
	SoundRender_Core_Processor.cpp
	SoundRender_Core_SourceManager.cpp
	SoundRender_Core_StartStop.cpp
	SoundRender_Emitter.cpp
	SoundRender_Emitter_FSM.cpp
	SoundRender_Emitter_StartStop.cpp
	SoundRender_Emitter_streamer.cpp
	SoundRender_Environment.cpp
	SoundRender_Source.cpp
	SoundRender_Source_loader.cpp
	SoundRender_Target.cpp
	SoundRender_TargetA.cpp
	stdafx.cpp
)

# Windows-only: guids.cpp instantiates DirectSound GUIDs, NotificationClient.cpp
# listens for audio device changes through Win32 COM
set_source_files_properties(guids.cpp NotificationClient.cpp PROPERTIES HEADER_FILE_ONLY ON)

target_compile_options(xrSound PUBLIC "-iquote${CMAKE_CURRENT_SOURCE_DIR}")
target_compile_definitions(xrSound PRIVATE XRSOUND_EXPORTS)
target_link_libraries(xrSound PUBLIC xrCore xrCDB xrAPI PkgConfig::VORBIS PkgConfig::OPENAL)
