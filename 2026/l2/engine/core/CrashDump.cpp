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
		LONG WINAPI OnUnhandledException(EXCEPTION_POINTERS* _exception_pointers)
		{
			static LONG s_handling = 0;
			if (InterlockedCompareExchange(&s_handling, 1, 0) != 0)
			{
				return EXCEPTION_EXECUTE_HANDLER;
			}

			wchar_t appdata_path[MAX_PATH]{};
			const DWORD appdata_length = GetEnvironmentVariableW(L"APPDATA", appdata_path, MAX_PATH);

			wchar_t crash_dump_file[MAX_PATH]{};
			BOOL dump_written = FALSE;
			DWORD dump_error = ERROR_SUCCESS;

			if (appdata_length > 0 && appdata_length < MAX_PATH)
			{
				SYSTEMTIME local_time{};
				GetLocalTime(&local_time);

				wchar_t crash_dump_directory[MAX_PATH]{};
				swprintf_s(crash_dump_directory, L"%s\\uwr", appdata_path);
				CreateDirectoryW(crash_dump_directory, nullptr);
				swprintf_s(crash_dump_directory, L"%s\\uwr\\dumps", appdata_path);
				CreateDirectoryW(crash_dump_directory, nullptr);

				swprintf_s(crash_dump_file, L"%s\\crash_dump_%04u-%02u-%02u_%02u-%02u-%02u.dmp",
					crash_dump_directory,
					local_time.wYear, local_time.wMonth, local_time.wDay,
					local_time.wHour, local_time.wMinute, local_time.wSecond);

				const HANDLE file_handle = CreateFileW(crash_dump_file, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

				if (file_handle != INVALID_HANDLE_VALUE)
				{
					MINIDUMP_EXCEPTION_INFORMATION exception_info;
					exception_info.ThreadId = GetCurrentThreadId();
					exception_info.ExceptionPointers = _exception_pointers;
					exception_info.ClientPointers = FALSE;

					constexpr MINIDUMP_TYPE dump_type = static_cast<MINIDUMP_TYPE>(
						MiniDumpWithDataSegs |
						MiniDumpWithHandleData |
						MiniDumpWithThreadInfo |
						MiniDumpWithUnloadedModules |
						MiniDumpWithIndirectlyReferencedMemory);

					dump_written = MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), file_handle, dump_type, &exception_info, nullptr, nullptr);
					dump_error = dump_written ? ERROR_SUCCESS : GetLastError();

					FlushFileBuffers(file_handle);
					CloseHandle(file_handle);
				}
				else
				{
					dump_error = GetLastError();
				}
			}

			LOG_CRITICAL("Crash! Exception 0x{:08X} at {}.", _exception_pointers->ExceptionRecord->ExceptionCode, _exception_pointers->ExceptionRecord->ExceptionAddress);

			const std::filesystem::path crash_dump_path_for_log(crash_dump_file);
			if (dump_written)
			{
				LOG_CRITICAL("Crash dump generated at: {}", crash_dump_path_for_log.string());
			}
			else
			{
				LOG_CRITICAL("Crash dump FAILED. Error/HRESULT: 0x{:08X}. File: {}", dump_error, crash_dump_path_for_log.string());
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
