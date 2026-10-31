// Error reporting and crash handling for non-Windows builds.
// Replaces xrDebugNew.cpp, which is built on SEH, minidumps and Win32 dialogs.
#include "stdafx.h"
#pragma hdrstop

#ifndef _WIN32

#include "xrdebug.h"
#include "os_clipboard.h"

#include <execinfo.h>
#include <signal.h>
#include <new>

extern bool shared_str_initialized;

XRCORE_API xrDebug Debug;

namespace crash_saving
{
	void (*save_impl)() = nullptr;
	BOOL enabled = TRUE;

	void save()
	{
		if (enabled && save_impl != nullptr)
		{
			(*save_impl)();
		}
	}
}

extern void printLuaStack();

void LogStackTrace(LPCSTR header = nullptr, bool printStack = false)
{
	if (!shared_str_initialized)
		return;

	if (header)
		Msg("%s", header);

	if (printStack)
	{
		printLuaStack();
		Msg("\n");

		void* frames[64];
		int count = backtrace(frames, 64);
		char** symbols = backtrace_symbols(frames, count);
		if (symbols)
		{
			// skip LogStackTrace itself
			for (int i = 1; i < count; ++i)
				Msg("%s", symbols[i]);
			free(symbols);
		}
	}
}

void LogStackTrace(LPCSTR header)
{
	LogStackTrace(header, false);
}

void xrDebug::gather_info(const char* expression, const char* description, const char* argument0, const char* argument1,
                          const char* file, int line, const char* function, LPSTR assertion_info,
                          u32 const assertion_info_size)
{
	if (!expression)
		expression = "<no expression>";
	LPSTR buffer_base = assertion_info;
	LPSTR buffer = assertion_info;
	int assertion_size = (int)assertion_info_size;
	LPCSTR endline = "\n";
	LPCSTR prefix = "[error]";
	bool extended_description = (description && !argument0 && strchr(description, '\n'));
	for (int i = 0; i < 2; ++i)
	{
		if (!i)
			buffer += xr_sprintf(buffer, assertion_size - u32(buffer - buffer_base), "%sFATAL ERROR%s%s", endline,
			                     endline, endline);
		buffer += xr_sprintf(buffer, assertion_size - u32(buffer - buffer_base), "%sExpression    : %s%s", prefix,
		                     expression, endline);
		buffer += xr_sprintf(buffer, assertion_size - u32(buffer - buffer_base), "%sFunction      : %s%s", prefix,
		                     function, endline);
		buffer += xr_sprintf(buffer, assertion_size - u32(buffer - buffer_base), "%sFile          : %s%s", prefix, file,
		                     endline);
		buffer += xr_sprintf(buffer, assertion_size - u32(buffer - buffer_base), "%sLine          : %d%s", prefix, line,
		                     endline);

		if (extended_description)
		{
			buffer += xr_sprintf(buffer, assertion_size - u32(buffer - buffer_base), "%s%s%s", endline, description,
			                     endline);
			if (argument0)
			{
				buffer += xr_sprintf(buffer, assertion_size - u32(buffer - buffer_base), "%s%s", argument0, endline);
				if (argument1)
					buffer += xr_sprintf(buffer, assertion_size - u32(buffer - buffer_base), "%s%s", argument1,
					                     endline);
			}
		}
		else
		{
			buffer += xr_sprintf(buffer, assertion_size - u32(buffer - buffer_base), "%sDescription   : %s%s", prefix,
			                     description, endline);
			if (argument0)
			{
				if (argument1)
				{
					buffer += xr_sprintf(buffer, assertion_size - u32(buffer - buffer_base), "%sArgument 0    : %s%s",
					                     prefix, argument0, endline);
					buffer += xr_sprintf(buffer, assertion_size - u32(buffer - buffer_base), "%sArgument 1    : %s%s",
					                     prefix, argument1, endline);
				}
				else
					buffer += xr_sprintf(buffer, assertion_size - u32(buffer - buffer_base), "%sArguments     : %s%s",
					                     prefix, argument0, endline);
			}
		}

		buffer += xr_sprintf(buffer, assertion_size - u32(buffer - buffer_base), "%s", endline);
		if (!i)
		{
			if (shared_str_initialized)
			{
				Msg("%s", assertion_info);
				FlushLog();
			}
			buffer = assertion_info;
			endline = "\n";
			prefix = "";
		}
	}

	if (!strstr(GetCommandLine(), "-no_call_stack_assert"))
	{
		if (shared_str_initialized)
			Msg("stack trace:\n");

		LogStackTrace(nullptr, true);

		if (shared_str_initialized)
			FlushLog();

		os_clipboard::copy_to_clipboard(assertion_info);
	}
}

void xrDebug::do_exit(const std::string& message)
{
	FlushLog();
	MessageBox(NULL, message.c_str(), "Error", MB_OK | MB_ICONERROR | MB_SYSTEMMODAL);
	_exit(1);
}

void xrDebug::backend(const char* expression, const char* description, const char* argument0, const char* argument1,
                      const char* file, int line, const char* function, bool& /*ignore_always*/)
{
	// save the game before anything else can go wrong
	crash_saving::save();

	static xrCriticalSection CS;
	CS.Enter();

	string4096 assertion_info;
	gather_info(expression, description, argument0, argument1, file, line, function, assertion_info,
	            sizeof(assertion_info));

	if (handler)
		handler();

	FlushLog();
	MessageBox(NULL, assertion_info, "Fatal Error", MB_OK | MB_ICONERROR | MB_SYSTEMMODAL);

	CS.Leave();
	_exit(1);
}

LPCSTR xrDebug::error2string(long code)
{
	static string1024 desc_storage;
	xr_strcpy(desc_storage, strerror((int)code));
	return desc_storage;
}

void xrDebug::error(long hr, const char* expr, const char* file, int line, const char* function, bool& ignore_always)
{
	backend(expr, error2string(hr), 0, 0, file, line, function, ignore_always);
}

void xrDebug::error(long hr, const char* expr, const char* e2, const char* file, int line, const char* function,
                    bool& ignore_always)
{
	backend(expr, error2string(hr), e2, 0, file, line, function, ignore_always);
}

void xrDebug::fail(const char* e1, const char* file, int line, const char* function, bool& ignore_always)
{
	backend(e1, "assertion failed", 0, 0, file, line, function, ignore_always);
}

void xrDebug::fail(const char* e1, const std::string& e2, const char* file, int line, const char* function,
                   bool& ignore_always)
{
	backend(e1, e2.c_str(), 0, 0, file, line, function, ignore_always);
}

void xrDebug::fail(const char* e1, const char* e2, const char* file, int line, const char* function,
                   bool& ignore_always)
{
	backend(e1, e2, 0, 0, file, line, function, ignore_always);
}

void xrDebug::fail(const char* e1, const char* e2, const char* e3, const char* file, int line, const char* function,
                   bool& ignore_always)
{
	backend(e1, e2, e3, 0, file, line, function, ignore_always);
}

void xrDebug::fail(const char* e1, const char* e2, const char* e3, const char* e4, const char* file, int line,
                   const char* function, bool& ignore_always)
{
	backend(e1, e2, e3, e4, file, line, function, ignore_always);
}

bool ignore_verify = true;

void xrDebug::soft_fail(LPCSTR e1, LPCSTR file, int line, LPCSTR function)
{
	if (!ignore_verify)
		Msg("! VERIFY_FAILED: %s[%d] {%s}  %s", file, line, function, e1);
}

void xrDebug::soft_fail(LPCSTR e1, const std::string& e2, LPCSTR file, int line, LPCSTR function)
{
	if (!ignore_verify)
		Msg("! VERIFY_FAILED: %s[%d] {%s}  %s %s", file, line, function, e1, e2.c_str());
}

void xrDebug::soft_fail(LPCSTR e1, LPCSTR e2, LPCSTR file, int line, LPCSTR function)
{
	if (!ignore_verify)
		Msg("! VERIFY_FAILED: %s[%d] {%s}  %s %s", file, line, function, e1, e2);
}

void xrDebug::soft_fail(LPCSTR e1, LPCSTR e2, LPCSTR e3, LPCSTR file, int line, LPCSTR function)
{
	if (!ignore_verify)
		Msg("! VERIFY_FAILED: %s[%d] {%s}  %s %s %s", file, line, function, e1, e2, e3);
}

void xrDebug::soft_fail(LPCSTR e1, LPCSTR e2, LPCSTR e3, LPCSTR e4, LPCSTR file, int line, LPCSTR function)
{
	if (!ignore_verify)
		Msg("! VERIFY_FAILED: %s[%d] {%s}  %s %s %s %s", file, line, function, e1, e2, e3, e4);
}

void xrDebug::soft_fail(LPCSTR e1, LPCSTR e2, LPCSTR e3, LPCSTR e4, LPCSTR e5, LPCSTR file, int line, LPCSTR function)
{
	if (!ignore_verify)
		Msg("! VERIFY_FAILED: %s[%d] {%s}  %s %s %s %s %s", file, line, function, e1, e2, e3, e4, e5);
}

void xrDebug::fatal(const char* file, int line, const char* function, const char* F, ...)
{
	string1024 buffer;

	va_list p;
	va_start(p, F);
	vsnprintf(buffer, sizeof(buffer), F, p);
	va_end(p);

	bool ignore_always = true;
	backend(nullptr, "fatal error", buffer, 0, file, line, function, ignore_always);
}

typedef void (*full_memory_stats_callback_type)();
XRCORE_API full_memory_stats_callback_type g_full_memory_stats_callback = 0;
XRCORE_API string_path g_bug_report_file;

static void out_of_memory_handler()
{
	Msg("* [x-ray]: out of memory");

	if (g_full_memory_stats_callback)
		g_full_memory_stats_callback();
	else
	{
		Memory.mem_compact();
		u32 eco_strings_count = 0;
		int eco_strings = (int)g_pStringContainer->stat_economy(eco_strings_count);
		int eco_smem = (int)g_pSharedMemoryContainer->stat_economy();
		Msg("* [x-ray]: strings: memory[%d K], count[%u]", eco_strings / 1024, eco_strings_count);
		Msg("* [x-ray]: shared: memory[%d K]", eco_smem);
	}

	Debug.fatal(DEBUG_INFO, "Out of memory");
}

static void handler_base(LPCSTR reason_string)
{
	bool ignore_always = false;
	Debug.backend(nullptr, reason_string, 0, 0, DEBUG_INFO, ignore_always);
}

static void signal_handler(int sig)
{
	// restore the default action so a fault inside the handler terminates the process
	signal(sig, SIG_DFL);
	switch (sig)
	{
	case SIGABRT: handler_base("application is aborting"); break;
	case SIGFPE: handler_base("floating point error"); break;
	case SIGILL: handler_base("illegal instruction"); break;
	case SIGSEGV:
	case SIGBUS: handler_base("illegal storage access"); break;
	case SIGTERM: handler_base("termination with exit code 3"); break;
	default: handler_base("unhandled signal"); break;
	}
}

void debug_on_thread_spawn()
{
	signal(SIGABRT, signal_handler);
	signal(SIGFPE, signal_handler);
	signal(SIGILL, signal_handler);
	signal(SIGSEGV, signal_handler);
	signal(SIGBUS, signal_handler);
	signal(SIGTERM, signal_handler);
	std::set_new_handler(&out_of_memory_handler);
}

void xrDebug::_initialize(const bool& /*dedicated*/)
{
	*g_bug_report_file = 0;
	debug_on_thread_spawn();
}

#endif // !_WIN32
