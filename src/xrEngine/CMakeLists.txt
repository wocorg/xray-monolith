# Source list generated from xrEngine.vcxproj (ReleaseR4|x64) by tools/vcxproj-sources.ps1.
# On Windows xrEngine is the game executable; here it is a library for now.
add_library(xrEngine STATIC
	"../3rd party/discord/achievement_manager.cpp"
	"../3rd party/discord/activity_manager.cpp"
	"../3rd party/discord/application_manager.cpp"
	"../3rd party/discord/core.cpp"
	"../3rd party/discord/image_manager.cpp"
	"../3rd party/discord/lobby_manager.cpp"
	"../3rd party/discord/network_manager.cpp"
	"../3rd party/discord/overlay_manager.cpp"
	"../3rd party/discord/relationship_manager.cpp"
	"../3rd party/discord/storage_manager.cpp"
	"../3rd party/discord/store_manager.cpp"
	"../3rd party/discord/types.cpp"
	"../3rd party/discord/user_manager.cpp"
	"../3rd party/discord/voice_manager.cpp"
	"ai_script_lua_debug.cpp"
	"ai_script_lua_extension.cpp"
	"bone.cpp"
	"CameraBase.cpp"
	"CameraManager.cpp"
	"cf_dynamic_mesh.cpp"
	"CustomHUD.cpp"
	"defines.cpp"
	"device.cpp"
	"GIFResource.cpp"
	"MonitorList.cpp"
	"Device_create.cpp"
	"Device_destroy.cpp"
	"Device_Initialize.cpp"
	"Device_Misc.cpp"
	"Device_overdraw.cpp"
	"Device_wndproc.cpp"
	"edit_actions.cpp"
	"Effector.cpp"
	"EffectorPP.cpp"
	"Engine.cpp"
	"EngineAPI.cpp"
	"Envelope.cpp"
	"Environment.cpp"
	"Environment_misc.cpp"
	"Environment_render.cpp"
	"EventAPI.cpp"
	"FDemoPlay.cpp"
	"FDemoRecord.cpp"
	"Feel_Touch.cpp"
	"Feel_Vision.cpp"
	"fmesh.cpp"
	"GameFont.cpp"
	"GameMtlLib.cpp"
	"GameMtlLib_Engine.cpp"
	"ICollidable.cpp"
	"IGame_Level.cpp"
	"IGame_Level_check_textures.cpp"
	"IGame_ObjectPool.cpp"
	"IGame_Persistent.cpp"
	"IInputReceiver.cpp"
	"imgui_base.cpp"
	"imgui_base_input.cpp"
	"imgui_helper.cpp"
	"interp.cpp"
	"IRenderable.cpp"
	"ISheduled.cpp"
	"LightAnimLibrary.cpp"
	"line_editor.cpp"
	"line_edit_control.cpp"
	"mailSlot.cpp"
	"MbHelpers.cpp"
	"motion.cpp"
	"ObjectAnimator.cpp"
	"ObjectDump.cpp"
	"perlin.cpp"
	"phdebug.cpp"
	"PS_instance.cpp"
	"pure.cpp"
	"pure_relcase.cpp"
	"Rain.cpp"
	"Render.cpp"
	"shader_bus.cpp"
	"SkeletonMotions.cpp"
	"StatGraph.cpp"
	"Stats.cpp"
	"stdafx.cpp"
	"Text_Console.cpp"
	"Text_Console_WndProc.cpp"
	"thunderbolt.cpp"
	"tntQAVI.cpp"
	"xrHemisphere.cpp"
	"xrImage_Resampler.cpp"
	"xrSASH.cpp"
	"xrSheduler.cpp"
	"xrTheora_Stream.cpp"
	"xrTheora_Surface.cpp"
	"xrTheora_Surface_mmx.cpp"
	"xr_collide_form.cpp"
	"xr_efflensflare.cpp"
	"Xr_input.cpp"
	"xr_input_xinput.cpp"
	"XR_IOConsole.cpp"
	"XR_IOConsole_callback.cpp"
	"XR_IOConsole_control.cpp"
	"XR_IOConsole_get.cpp"
	"xr_ioc_cmd.cpp"
	"xr_object.cpp"
	"xr_object_list.cpp"
	"x_ray.cpp"
	"_scripting.cpp"
)

# Discord Rich Presence needs the Windows Discord SDK library
file(GLOB XRENGINE_DISCORD_SOURCES "${XRAY_SRC_DIR}/3rd party/discord/*.cpp")
set_source_files_properties(${XRENGINE_DISCORD_SOURCES} PROPERTIES HEADER_FILE_ONLY ON)

# Win32-only parts, not built on macOS. TODO(macos-port): replace with SDL in the
# window/input stage: DirectInput/XInput input, window creation and message handling,
# the dedicated-server text console, monitor enumeration, keyboard layouts for ImGui,
# AVI playback.
set_source_files_properties(
	Xr_input.cpp xr_input_xinput.cpp
	Device_create.cpp Device_destroy.cpp Device_Initialize.cpp Device_wndproc.cpp
	Text_Console.cpp Text_Console_WndProc.cpp
	MonitorList.cpp imgui_base_input.cpp tntQAVI.cpp
	PROPERTIES HEADER_FILE_ONLY ON)

target_compile_options(xrEngine PUBLIC "-iquote${CMAKE_CURRENT_SOURCE_DIR}")
target_include_directories(xrEngine PRIVATE
	"${XRAY_SRC_DIR}/3rd party/imgui"
	"${XRAY_SRC_DIR}/3rd party/luabind"
	"${XRAY_SRC_DIR}/3rd party/giflib"
	"${XRAY_SRC_DIR}/3rd party/icu/include"
)
target_compile_definitions(xrEngine PRIVATE ENGINE_BUILD)
target_compile_options(xrEngine PRIVATE ${XRAY_MSVC_COMPAT_FLAGS})
target_link_libraries(xrEngine PUBLIC xrCore xrCDB xrAPI xrNetServer xrSound xrParticles xrPhysics xrXMLParser xrCPU_Pipe
	luabind ode PkgConfig::THEORA)