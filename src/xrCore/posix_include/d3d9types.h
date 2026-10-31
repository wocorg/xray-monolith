// macOS stand-in for the MSVC/Windows header of the same name
#pragma once
// only the colour helpers used outside the renderer
typedef DWORD D3DCOLOR;
#define D3DCOLOR_ARGB(a, r, g, b) ((D3DCOLOR)((((a) & 0xff) << 24) | (((r) & 0xff) << 16) | (((g) & 0xff) << 8) | ((b) & 0xff)))
#define D3DCOLOR_RGBA(r, g, b, a) D3DCOLOR_ARGB(a, r, g, b)
#define D3DCOLOR_XRGB(r, g, b) D3DCOLOR_ARGB(0xff, r, g, b)
#define D3DCOLOR_COLORVALUE(r, g, b, a) D3DCOLOR_RGBA((DWORD)((r) * 255.f), (DWORD)((g) * 255.f), (DWORD)((b) * 255.f), (DWORD)((a) * 255.f))
