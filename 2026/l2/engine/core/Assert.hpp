#pragma once

#include <string_view>
#include <spdlog/fmt/fmt.h>

namespace engine::core
{
	[[noreturn]] void Assert(std::string_view _expression, std::string_view _message, const char* _file, int _line, const char* _function);
}

#if defined(_MSC_VER)
#define DEBUG_BREAK() __debugbreak()
#define FUNCTION __FUNCSIG__
#else
#define DEBUG_BREAK() __builtin_trap()
#define FUNCTION __PRETTY_FUNCTION__
#endif

#if defined(DEBUG)
#define ASSERT(expr, message)                                                   \
		do {                                                                        \
			if (!(expr)) {                                                            \
				::engine::core::Assert(#expr, message, __FILE__, __LINE__, FUNCTION);   \
			}                                                                         \
		} while (false)
#else
#define ASSERT(expr, message) ((void)0)
#endif