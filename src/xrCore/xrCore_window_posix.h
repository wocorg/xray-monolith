// Win32 window, cursor and keyboard-state functions used by engine and game code.
// TODO(macos-port): these are placeholders so the code compiles; they report "no
// window" / neutral values. The SDL window/input stage implements them for real
// (cursor position and clipping, client rectangles, screen metrics, key states).
#ifndef XRCORE_WINDOW_POSIX_H
#define XRCORE_WINDOW_POSIX_H
#pragma once

#define LOWORD(l) ((WORD)(((DWORD_PTR)(l)) & 0xffff))
#define HIWORD(l) ((WORD)((((DWORD_PTR)(l)) >> 16) & 0xffff))

#define WM_DESTROY 0x0002
#define WM_CLOSE 0x0010
#define WM_QUIT 0x0012
#define WM_COMMAND 0x0111
#define WA_INACTIVE 0

#define VK_CAPITAL 0x14
#define VK_NUMLOCK 0x90

#define SM_CXSCREEN 0
#define SM_CYSCREEN 1
#define SM_SWAPBUTTON 23

#define MONITOR_DEFAULTTONULL 0
#define MONITOR_DEFAULTTOPRIMARY 1
#define MONITOR_DEFAULTTONEAREST 2

#define HKL_NEXT ((HKL)1)

inline BOOL GetCursorPos(LPPOINT p) { p->x = p->y = 0; return FALSE; }
inline BOOL SetCursorPos(int, int) { return FALSE; }
inline BOOL ClientToScreen(HWND, LPPOINT) { return FALSE; }
inline BOOL ScreenToClient(HWND, LPPOINT) { return FALSE; }
inline int ShowCursor(BOOL show) { return show ? 0 : -1; }
inline BOOL ClipCursor(const RECT*) { return FALSE; }
inline BOOL GetClientRect(HWND, LPRECT r) { r->left = r->top = r->right = r->bottom = 0; return FALSE; }
inline int MapWindowPoints(HWND, HWND, LPPOINT, UINT) { return 0; }
inline int GetSystemMetrics(int) { return 0; }
inline short GetKeyState(int) { return 0; }
inline HKL ActivateKeyboardLayout(HKL, UINT) { return nullptr; }
inline HMONITOR MonitorFromPoint(POINT, DWORD) { return nullptr; }
inline HWND GetDesktopWindow() { return nullptr; }
inline BOOL SetForegroundWindow(HWND) { return FALSE; }
inline BOOL DestroyWindow(HWND) { return FALSE; }
inline void PostQuitMessage(int) {}

#endif // XRCORE_WINDOW_POSIX_H
