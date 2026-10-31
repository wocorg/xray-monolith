////////////////////////////////////////////////////////////////////////////
//	Module 		: script_render_device_script.cpp
//	Created 	: 28.06.2004
//  Modified 	: 28.06.2004
//	Author		: Dmitriy Iassenev
//	Description : Script render device script export
////////////////////////////////////////////////////////////////////////////

#include "pch_script.h"
#include "script_render_device.h"

using namespace luabind;

bool is_device_paused(CRenderDevice* d)
{
	return !!Device.Paused();
}

void set_device_paused(CRenderDevice* d, bool b)
{
	Device.Pause(b, TRUE, FALSE, "set_device_paused_script");
}

extern ENGINE_API BOOL bShowPauseString;

void set_device_paused_ex(CRenderDevice* d, bool b)
{
	Device.Pause(b, TRUE, TRUE, "set_device_paused_ex_script");
	bShowPauseString = FALSE;
}

extern ENGINE_API BOOL g_appLoaded;

bool is_app_ready()
{
	return !!g_appLoaded;
}

u32 time_global(const CRenderDevice* self)
{
	THROW(self);
	return (self->dwTimeGlobal);
}

u32 time_continual(const CRenderDevice* self)
{
	THROW(self);
	return (self->dwTimeContinual);
}

// fields live in the CRenderDeviceData base; luabind wants members of CRenderDevice itself
#define XR_DEVICE_FIELD(field) static_cast<decltype(CRenderDevice::field) CRenderDevice::*>(&CRenderDevice::field)

#pragma optimize("s",on)
template <> void CScriptRenderDevice::script_register(lua_State* L)
{
	module(L)
	[
		class_<CRenderDevice>("render_device")
		.def_readonly("width", XR_DEVICE_FIELD(dwWidth))
		.def_readonly("height", XR_DEVICE_FIELD(dwHeight))
		.def_readonly("time_delta", XR_DEVICE_FIELD(dwTimeDelta))
		.def_readonly("f_time_delta", XR_DEVICE_FIELD(fTimeDelta))
		.def_readonly("cam_pos", XR_DEVICE_FIELD(vCameraPosition))
		.def_readonly("cam_dir", XR_DEVICE_FIELD(vCameraDirection))
		.def_readonly("cam_top", XR_DEVICE_FIELD(vCameraTop))
		.def_readonly("cam_right", XR_DEVICE_FIELD(vCameraRight))
		//			.def_readonly("view",					XR_DEVICE_FIELD(mView))
		//			.def_readonly("projection",				XR_DEVICE_FIELD(mProject))
		//			.def_readonly("full_transform",			XR_DEVICE_FIELD(mFullTransform))
		.def_readonly("fov", XR_DEVICE_FIELD(fFOV))
		.def_readonly("aspect_ratio", XR_DEVICE_FIELD(fASPECT))
		.def("time_global", &time_global)
		.def("time_continual", &time_continual)
		.def_readonly("precache_frame", XR_DEVICE_FIELD(dwPrecacheFrame))
		.def_readonly("frame", XR_DEVICE_FIELD(dwFrame))
		.def("is_paused", &is_device_paused)
		.def("pause", &set_device_paused)
		.def("pause_ex", &set_device_paused_ex),
		def("app_ready", &is_app_ready)
	];
}
