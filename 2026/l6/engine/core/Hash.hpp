#pragma once

#include <cstdint>
#include <string_view>
#include <type_traits>

namespace engine::core
{
	///////////////////////////////////////////////////////////////////////////////////////
	template<typename T>
	struct SFnv1aTraits;

	template<>
	struct SFnv1aTraits<uint32_t>
	{
		static constexpr uint32_t Offset = 2166136261u;
		static constexpr uint32_t Prime = 16777619u;
	};

	template<>
	struct SFnv1aTraits<uint64_t>
	{
		static constexpr uint64_t Offset = 14695981039346656037ull;
		static constexpr uint64_t Prime = 1099511628211ull;
	};

	///////////////////////////////////////////////////////////////////////////////////////
	template<typename T>
	[[nodiscard]] constexpr T Fnv1a(std::string_view _str)
	{
		static_assert(std::is_integral_v<T> && std::is_unsigned_v<T>, "Fnv1a only supports unsigned integral types");

		T hash = SFnv1aTraits<T>::Offset;

		for (const unsigned char c : _str)
		{
			hash ^= static_cast<T>(c);
			hash *= SFnv1aTraits<T>::Prime;
		}

		return hash;
	}

	[[nodiscard]] constexpr uint32_t Fnv1a32(std::string_view _str) { return Fnv1a<uint32_t>(_str); }
	[[nodiscard]] constexpr uint64_t Fnv1a64(std::string_view _str) { return Fnv1a<uint64_t>(_str); }
}