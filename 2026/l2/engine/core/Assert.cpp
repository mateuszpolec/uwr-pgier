#include <cstdlib>

#if defined(WINDOWS)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <DbgHelp.h>
#pragma comment(lib, "Dbghelp.lib")
#endif

#include "Assert.hpp"
#include "Logger.hpp"

namespace engine::core
{
	void Assert(std::string_view expression, std::string_view message, const char* file, int line, const char* function)
	{
		LOG_ERROR("Assertion failed: {}. Message: {}. File: {}. Line: {}. Function: {}", expression, message, file, line, function);

#if defined(WINDOWS)
		static constexpr int MAX_CALLSTACK_DEPTH = 64;
		void* callstack[MAX_CALLSTACK_DEPTH];
		const unsigned short frames = CaptureStackBackTrace(0, MAX_CALLSTACK_DEPTH, callstack, nullptr);

		alignas(SYMBOL_INFO) char buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(char)]{};
		SYMBOL_INFO* symbol = reinterpret_cast<SYMBOL_INFO*>(buffer);
		symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
		symbol->MaxNameLen = MAX_SYM_NAME;

		const HANDLE process = GetCurrentProcess();
		SymSetOptions(SymGetOptions() | SYMOPT_LOAD_LINES | SYMOPT_UNDNAME);
		SymInitialize(process, nullptr, TRUE);

		IMAGEHLP_LINE64 line_info{};
		line_info.SizeOfStruct = sizeof(IMAGEHLP_LINE64);

		for (unsigned short i = 1; i < frames; i++)
		{
			const DWORD64 address = reinterpret_cast<DWORD64>(callstack[i]);
			const char* name = SymFromAddr(process, address, nullptr, symbol) ? symbol->Name : "<unknown>";

			DWORD displacement = 0;
			if (SymGetLineFromAddr64(process, address, &displacement, &line_info))
			{
				LOG_INFO("  #{}: {} - 0x{:016X} ({}:{})", i - 1, name, address, line_info.FileName, line_info.LineNumber);
			}
			else
			{
				LOG_INFO("  #{}: {} - 0x{:016X}", i - 1, name, address);
			}
		}

		SymCleanup(process);

		if (IsDebuggerPresent())
		{
			DEBUG_BREAK();
		}
#else
		DEBUG_BREAK();
#endif

		std::abort();
	}
}