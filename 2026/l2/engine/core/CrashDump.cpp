#include <chrono>

#if defined(WINDOWS)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <DbgHelp.h>
#pragma comment(lib, "Dbghelp.lib")
#endif

#include "CrashDump.hpp"
#include "Logger.hpp"

#include "engine/filesystem/Paths.hpp"

namespace engine::core
{
	namespace
	{
#if defined(WINDOWS)
		LONG WINAPI OnUnhandledException(EXCEPTION_POINTERS* exception_pointers)
		{
			LOG_CRITICAL("Unhandled exception occurred. Generating crash dump...");

			const std::filesystem::path crash_dump_path = engine::filesystem::AppData() / "uwr" / "dumps";

			const auto now = std::chrono::current_zone()->to_local(std::chrono::system_clock::now());
			const std::filesystem::path crash_dump_file = crash_dump_path / std::format("crash_dump_{:%Y-%m-%d_%H-%M-%S}.dmp", std::chrono::floor<std::chrono::seconds>(now));

			if (!std::filesystem::exists(crash_dump_path))
			{
				std::filesystem::create_directories(crash_dump_path);
			}

			HANDLE file_handle = CreateFileW(crash_dump_file.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

			LOG_CRITICAL("Crash! Exception 0x{:08X} at {}. Minidump: {}", exception_pointers->ExceptionRecord->ExceptionCode, exception_pointers->ExceptionRecord->ExceptionAddress, crash_dump_file.string());

			if (file_handle != INVALID_HANDLE_VALUE)
			{
				MINIDUMP_EXCEPTION_INFORMATION exception_info;
				exception_info.ThreadId = GetCurrentThreadId();
				exception_info.ExceptionPointers = exception_pointers;
				exception_info.ClientPointers = FALSE;
				MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), file_handle, MiniDumpWithFullMemory, &exception_info, nullptr, nullptr);
				CloseHandle(file_handle);
				LOG_CRITICAL("Crash dump generated at: {}", crash_dump_file.string());
			}
			else
			{
				LOG_CRITICAL("Failed to create crash dump file: {}", crash_dump_file.string());
			}

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

			spdlog::default_logger()->flush();
			return EXCEPTION_EXECUTE_HANDLER;
		}
#endif
	}

	void InitializeCrashDump()
	{
#if defined(WINDOWS)
		SetUnhandledExceptionFilter(OnUnhandledException);
#endif
	}
}
