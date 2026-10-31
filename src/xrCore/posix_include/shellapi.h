// macOS stand-in for the Windows header of the same name
#pragma once
#include "../xrCore_platform.h"

#define SW_HIDE 0
#define SW_SHOW 5

// The only caller starts cmd.exe, which does not exist on macOS
inline HINSTANCE ShellExecute(HWND, LPCSTR, LPCSTR, LPCSTR, LPCSTR, INT) { return nullptr; }
