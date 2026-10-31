////////////////////////////////////////////////////////////////////////////
// Module : os_clipboard.cpp
// Created : 21.02.2008
// Author : Evgeniy Sokolov
// Description : os clipboard class implementation
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#pragma hdrstop
#include "os_clipboard.h"

#ifndef _WIN32
// macOS: the system pasteboard through pbcopy/pbpaste
void os_clipboard::copy_to_clipboard(LPCSTR buf)
{
	FILE* pipe = popen("pbcopy", "w");
	if (!pipe)
		return;
	fputs(buf, pipe);
	pclose(pipe);
}

void os_clipboard::paste_from_clipboard(LPSTR buffer, u32 const& buffer_size)
{
	VERIFY(buffer);
	VERIFY(buffer_size > 0);

	buffer[0] = 0;
	FILE* pipe = popen("pbpaste", "r");
	if (!pipe)
		return;
	size_t n = fread(buffer, 1, buffer_size - 1, pipe);
	pclose(pipe);
	buffer[n] = 0;
	for (u32 i = 0; i < n; ++i)
	{
		char c = buffer[i];
		if (((isprint(c) == 0) && (c != char(-1))) || c == '\t' || c == '\n')
			buffer[i] = ' ';
	}
}

void os_clipboard::update_clipboard(LPCSTR string)
{
	string4096 current;
	u32 size = sizeof(current);
	paste_from_clipboard(current, size);
	xr_string combined = current;
	combined += string;
	copy_to_clipboard(combined.c_str());
}
#else
void os_clipboard::copy_to_clipboard(LPCSTR buf)
{
	if (!OpenClipboard(0))
		return;
	u32 handle_size = (xr_strlen(buf) + 1) * sizeof(char);
	HGLOBAL handle = GlobalAlloc(GHND, handle_size);
	if (!handle)
	{
		CloseClipboard();
		return;
	}

	char* memory = (char*)GlobalLock(handle);
	xr_strcpy(memory, handle_size, buf);
	GlobalUnlock(handle);
	EmptyClipboard();
	SetClipboardData(CF_TEXT, handle);
	CloseClipboard();
}

void os_clipboard::paste_from_clipboard(LPSTR buffer, u32 const& buffer_size)
{
	VERIFY(buffer);
	VERIFY(buffer_size > 0);

	if (!OpenClipboard(0))
		return;

	HGLOBAL hmem = GetClipboardData(CF_TEXT);
	if (!hmem)
		return;

	LPCSTR clipdata = (LPCSTR)GlobalLock(hmem);
	strncpy_s(buffer, buffer_size, clipdata, buffer_size - 1);
	buffer[buffer_size - 1] = 0;
	for (u32 i = 0; i < strlen(buffer); ++i)
	{
		char c = buffer[i];
		if (((isprint(c) == 0) && (c != char(-1))) || c == '\t' || c == '\n') // "я" = -1
		{
			buffer[i] = ' ';
		}
	}

	GlobalUnlock(hmem);
	CloseClipboard();
}

void os_clipboard::update_clipboard(LPCSTR string)
{
	if (!OpenClipboard(0))
		return;

	HGLOBAL handle = GetClipboardData(CF_TEXT);
	if (!handle)
	{
		CloseClipboard();
		copy_to_clipboard(string);
		return;
	}

	LPSTR memory = (LPSTR)GlobalLock(handle);
	int memory_length = (int)strlen(memory);
	int string_length = (int)strlen(string);
	int buffer_size = (memory_length + string_length + 1) * sizeof(char);
#ifndef _EDITOR
	LPSTR buffer = (LPSTR)_alloca(buffer_size);
#else // #ifndef _EDITOR
    LPSTR buffer = (LPSTR)xr_alloc<char>( buffer_size );
#endif // #ifndef _EDITOR
	xr_strcpy(buffer, buffer_size, memory);
	GlobalUnlock(handle);

	xr_strcat(buffer, buffer_size, string);
	CloseClipboard();
	copy_to_clipboard(buffer);
#ifdef _EDITOR
    xr_free (buffer);
#endif // #ifdef _EDITOR
}
#endif // _WIN32
